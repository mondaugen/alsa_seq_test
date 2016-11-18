/* Simple program that opens a sequencer output port and repreatedly sends
 * note on messages on channel 1 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h> 
#include <alsa/asoundlib.h> 

#define ERR_EXIT(str)\
    fprintf(stderr,"Error: %s",str);\
    return -1

typedef void (*sighandler_t) (int);

static volatile int done = 0;


void sigint_handler(int sn)
{
    done = 1;
}

snd_seq_t *ast_open_client(void)
{
    snd_seq_t *handle;
    int err;
    err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_OUTPUT, 0);
    if (err < 0) {
        return NULL;
    }
    snd_seq_set_client_name(handle, "My Client");
    return handle;
}

int ast_new_port(snd_seq_t *handle)
{
    return snd_seq_create_simple_port(handle, "my port",
            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_MIDI_GENERIC);
}

void ast_simple_noteon(snd_seq_t *seq, int port)
{
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_noteon(&ev,0,60,100);
        snd_seq_event_output(seq, &ev);
        snd_seq_drain_output(seq);
}

void ast_simple_noteoff(snd_seq_t *seq, int port)
{
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_source(&ev, port);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_noteoff(&ev,0,60,0);
        snd_seq_event_output(seq, &ev);
        snd_seq_drain_output(seq);
}

int main (void)
{
    snd_seq_t *ast_handle;
    int ast_port;
    int err;
    ast_handle = ast_open_client();
    if (!ast_handle) {
        ERR_EXIT("Opening client.");
    }
    ast_port = ast_new_port(ast_handle);
    if (ast_port < 0) {
        ERR_EXIT("Opening port.");
    }
    

    signal(SIGINT, sigint_handler);
    while (!done) {
        printf("send noteon\n");
        ast_simple_noteon(ast_handle,ast_port);
        sleep(1);
        printf("send noteoff\n");
        ast_simple_noteoff(ast_handle,ast_port);
        sleep(1);
    }

    err = snd_seq_delete_port(ast_handle, ast_port);
    if (err < 0) {
        ERR_EXIT("Closing port.");
    }
    err = snd_seq_close(ast_handle);
    if (err < 0 ) {
        ERR_EXIT("Closing client.");
    }
}
