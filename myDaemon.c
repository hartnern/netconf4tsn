#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <libyang/libyang.h>
#include <sysrepo.h>

#define DEVICE_NAME "enp4s0"
#define NUM_TC 4
#define MAP "2 2 1 0 2 2 2 2 2 2 2 2 2 2 2 2"
#define QUEUE "1@0 1@1 1@2 1@3"
#define FLAGS "0x2"
#define TXTIME_DELAY 0

struct sched_entry {
	uint32_t index;
    char* operation_name;
	uint32_t time_interval;
    uint8_t gate_state;
};

sr_session_ctx_t *sess;
sr_subscription_ctx_t *subscription;
volatile int exit_application = 0;

void
configure_card(uint64_t base_time, uint64_t cycle_time, struct sched_entry* sched_entries, size_t sched_entry_cnt)
{
    //create the command
    char* cmd = malloc(1000);
    memset(cmd, 0, 999);
    sprintf(cmd, "tc qdisc add dev %s parent root stab overhead 24 handle 100 taprio \\\nnum_tc %d \\\nmap %s \\\nqueues %s \\\nbase-time %ld \\\ncycle-time %ld \\\nflags %s \\\n", DEVICE_NAME, NUM_TC, MAP, QUEUE, base_time, cycle_time, FLAGS);
    for(int i=0; i<sched_entry_cnt; i++){
        sprintf(cmd+strlen(cmd), "sched-entry S %X %d \\\n", sched_entries[i].gate_state, sched_entries[i].time_interval);
    }

    sprintf(cmd+strlen(cmd), "txtime-delay %d \n", TXTIME_DELAY);
    
    printf("\nCMD:\n%s\n", cmd);

    //delete old config if one exists - we don't care about the result
    system("tc qdisc delete dev enp4s0 parent root > /dev/null 2>&1");

    //send new config
    if(system(cmd)==0){
        printf("Configuration successfull\n");
    } else{
        printf("Error writing to card\n");
    }

    //clean up
    free(cmd);
}


static int
config_change_cb(sr_session_ctx_t *session, uint32_t sub_id, const char *module_name, const char *xpath,
        sr_event_t event, uint32_t request_id, void *private_data)
{
    uint64_t base_time=0;
    float cycle_time=0; 
    struct sched_entry* sched_entries = NULL;
    size_t sched_entry_cnt = 0;

    int rc;
    sr_val_t *val;
    size_t val_cnt;

    //get basetime
    rc = sr_get_item(session, "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-sched-bridge:gate-parameter-table/admin-base-time/seconds", 0, &val);
    if (rc != SR_ERR_OK) {
        goto sr_error;
    }
    base_time = (val->data.uint64_val)*10e9;

    rc = sr_get_item(session, "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-sched-bridge:gate-parameter-table/admin-base-time/nanoseconds", 0, &val);
    if (rc != SR_ERR_OK) {
        goto sr_error;
    }
    base_time += val->data.uint32_val;

    //get sched-entries
    rc = sr_get_items(session, "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-sched-bridge:gate-parameter-table/admin-control-list/gate-control-entry/*", 0, 0, &val, &val_cnt);
    if (rc != SR_ERR_OK) {
        goto sr_error;
    }
    sched_entry_cnt = val_cnt/4; 
    
    sched_entries = malloc(100*sched_entry_cnt);
    for (int i = 0; i < val_cnt; i++) {
        struct sched_entry entry;
        entry.index = val[i*4].data.uint32_val;
        entry.operation_name = val[i*4+1].data.string_val;
        entry.time_interval = val[i*4+2].data.uint32_val;
        entry.gate_state = val[i*4+3].data.uint8_val;
        sched_entries[i] = entry;
    }

    //get admin-cycle-time
    rc = sr_get_item(session, "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-sched-bridge:gate-parameter-table/admin-cycle-time/numerator", 0, &val);
    if (rc != SR_ERR_OK) {
        goto sr_error;
    }
    cycle_time = val->data.uint32_val;

    rc = sr_get_item(session, "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-sched-bridge:gate-parameter-table/admin-cycle-time/denominator", 0, &val);
    if (rc != SR_ERR_OK) {
        goto sr_error;
    }
    cycle_time = (cycle_time/val->data.uint32_val)*10e9;


    //set configuration to card and clean up
    configure_card(base_time, cycle_time, sched_entries, sched_entry_cnt);
    free(sched_entries);
    sr_free_val(val);

    return SR_ERR_OK;

sr_error:
    printf("Config change callback failed: %s.", sr_strerror(rc));
    return rc;
}

static void sigint_handler(__attribute__((unused)) int signum)
{
	printf("Sigint called, exiting...\n");
	exit_application = 1;
}

int main(int argc, char *argv[]) {

    sr_error_t err;
    sr_conn_ctx_t *connection = NULL;
    err = sr_connect(SR_CONN_DEFAULT, &connection);
    if(err==SR_ERR_OK){
        err = sr_session_start(connection, SR_DS_RUNNING ,&sess);
        if(err==SR_ERR_OK){
            err = sr_module_change_subscribe(sess, "ietf-interfaces", "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-sched-bridge:gate-parameter-table", config_change_cb, NULL, 0,
            SR_SUBSCR_ENABLED | SR_SUBSCR_DONE_ONLY, &subscription);
            if (err==SR_ERR_OK) {
                //printf("initialization successfull.\n");
                /* loop until ctrl-c is pressed / SIGINT is received */
                signal(SIGINT, sigint_handler);
                signal(SIGPIPE, SIG_IGN);
                while (!exit_application) {
                    sleep(1);
                }
                sr_unsubscribe(subscription);
            } else {
                printf("initialization failed: %s.\n", sr_strerror(err));
            } 
        }
    }
}