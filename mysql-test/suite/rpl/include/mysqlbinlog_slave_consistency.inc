# This file provides the structure to run a single test that ensures the
# mariadb-binlog command line tool is consistent with replicas for event
# filtering. The test is configured using the following input parameters, where
# each is nullable (i.e. it will not be used to configure mariadb-binlog or
# the replica).
#
# param $do_domain_ids     : A list of domain ids to include in replication
# param $ignore_domain_ids : A list of domain ids to exclude from replication
# param $ignore_server_ids : A list of server ids to exclude from replication
# param $start_position    : The GTID positions to begin replication from in
#                            the specified domains
# param $stop_position     : The GTID positions that mark the end of an event
#                            stream in a particular domain
#
# param $con1              : The connection name of the primary server
# param $con2              : The connection name of the replica server


--let $include_filename= mysqlbinlog_slave_consistency.inc
--source include/begin_include_file.inc

--enable_query_log

if (!$con1)
{
    --let $con1=master
}
if (!$con2)
{
    --let $con2=slave
}

--connection $con2
--source include/stop_slave.inc

--connection $con1
--echo # Populating $con1 data
--source include/sql_multisource.inc

--connection $con2
if (`SELECT strcmp("$start_position","") != 0`)
{
    eval set global gtid_slave_pos="$start_position";
}

--let $cm_args= MASTER_USE_GTID=slave_pos
--let $msbl_args=
if (`SELECT strcmp("$do_domain_ids","") != 0`)
{
    --let $cm_args= $cm_args, DO_DOMAIN_IDS=($do_domain_ids)
    --let $msbl_args= $msbl_args --do-domain-ids=$do_domain_ids
}
if (`SELECT strcmp("$ignore_domain_ids","") != 0`)
{
    --let $cm_args= $cm_args, IGNORE_DOMAIN_IDS=($ignore_domain_ids)
    --let $msbl_args= $msbl_args --ignore-domain-ids=$ignore_domain_ids
}
if (`SELECT strcmp("$ignore_server_ids","") != 0`)
{
    --let $cm_args= $cm_args, IGNORE_SERVER_IDS=($ignore_server_ids)
    --let $msbl_args= $msbl_args --ignore-server-ids=$ignore_server_ids
}

eval CHANGE MASTER TO $cm_args;

--let $start_slave_args=
if (`SELECT strcmp("$stop_position","") != 0`)
{
    --let $start_slave_args= UNTIL master_gtid_pos="$stop_position"
    --let $msbl_args= $msbl_args --stop-position=$stop_position
}

eval START SLAVE $start_slave_args;

# Stop position was not specified
if (`SELECT strcmp("$stop_position","") = 0`)
{
    --echo # Wait for $con2 to catch up to $con1

    --source include/wait_for_slave_to_start.inc
    --echo # Wait for $con2 IO thread to catch up
    --let $wait_condition= SELECT STATE="Waiting for master to send event" from information_schema.PROCESSLIST where COMMAND="Slave_IO"
    --source include/wait_condition.inc

    --echo # Wait for $con2 SQL thread to catch up
    --let $wait_condition= SELECT STATE="Slave has read all relay log; waiting for more updates" from information_schema.PROCESSLIST where COMMAND="Slave_SQL"
    --source include/wait_condition.inc

}

# Stop position was specified
if (`SELECT strcmp("$stop_position","") != 0`)
{
    --echo # Because there is a stop position we wait for all events to process
    --echo # and $con2 to automatically stop
    --source include/wait_for_slave_to_stop.inc
}

--echo # Stop $con2 so it stops receiving $con1 events.
--source include/stop_slave.inc

--echo # Use mysqlbinlog to update $con1 with the same parameters as $con2
--let $MYSQLD_DATADIR=`select @@datadir`
--let BINLOG_FILENAME= query_get_value(SHOW BINARY LOGS, Log_name, 1)
--let BINLOG_FILE_PARAM= $MYSQLD_DATADIR/$BINLOG_FILENAME.orig
--copy_file $MYSQLD_DATADIR/$BINLOG_FILENAME $BINLOG_FILE_PARAM

--connection $con1
DROP TABLE IF EXISTS t1, t2, t3, t4, t5;
RESET MASTER;
--echo # MYSQL_BINLOG BINLOG_FILE_PARAM $msbl_args | MYSQL
--exec $MYSQL_BINLOG $BINLOG_FILE_PARAM $msbl_args | $MYSQL

--source include/rpl_check_table_consistency.inc

--echo # Test finished - resetting $con1 and $con2..
--connection $con2
--source include/stop_slave.inc
RESET SLAVE;
set global gtid_slave_pos="";
CHANGE MASTER TO DO_DOMAIN_IDS=(), IGNORE_DOMAIN_IDS=(), IGNORE_SERVER_IDS=();

--connection $con1
RESET MASTER;
DROP TABLE IF EXISTS t1, t2, t3, t4, t5;
--source include/save_master_gtid.inc

--connection $con2
--source include/start_slave.inc
--source include/wait_for_slave_to_start.inc
--source include/sync_with_master_gtid.inc
--source include/stop_slave.inc
RESET SLAVE;
set global gtid_slave_pos="";
RESET MASTER;

--connection $con1
RESET MASTER;

--connection $con2
--source include/start_slave.inc

--connection $con1
--remove_file $BINLOG_FILE_PARAM

--let $include_filename= mysqlbinlog_slave_consistency.inc
--source include/end_include_file.inc