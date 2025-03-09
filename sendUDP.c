#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <libhackrf/hackrf.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5005
#define BUFFER 4096

int running = 1; //Flag to indicaite if running
hackrf_device *d; //Global device pointer
int sock;
struct sockaddr_in server_addr;
int16_t toUDP[BUFFER];

void sigint_handler(int signum){
    printf("\nSIGINT received. Stopping HackRF...\n");
    running = 0;
    if(d){
        hackrf_stop_rx(d);  // Stop receiving
        hackrf_close(d);    // Close HackRF
        hackrf_exit();      // Cleanup HackRF
        close(sock);
        printf("HackRF stopped and cleaned up.\n");
    }
    exit(0);
}

int callback(hackrf_transfer *transfer){
    int16_t *buf = (int16_t*) transfer->buffer;
    int n_samples = (transfer->valid_length / sizeof(int16_t))/2;
    printf("Number of samples from buffer: %d\n", n_samples);
    // Copy only required samples to avoid overflows
    int proc_samples = (n_samples < BUFFER) ? n_samples : BUFFER;
    memcpy(toUDP, buf, proc_samples * sizeof(int16_t));
    printf("Processed samples (int16): %d\n", (int)(proc_samples * sizeof(int16_t)));
    printf("Size of int16_t %d\n", sizeof(int16_t));
    // Send the processed samples over UDP
    sendto(sock, toUDP, proc_samples * sizeof(int16_t), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));
    return 0; 
}

int main(int argc, char *argv[]){
    //Get cli params
    //freq (MHz), sample rate (bandwidth), LNA gain
    if(argc < 2){
        printf("Include Freq (MHz), sample rate (MHz), LNA gain (dB)\n");
        return 1;
    }
    const int mhz = 1000000; 
    printf("Heh, starting......\n");
    long long int CENTER_FREQ = strtoll(argv[1], NULL, 10)*mhz;
    long long int SAMPLE_RATE = strtoll(argv[2], NULL, 10)*mhz;
    int LNA_GAIN = atoi(argv[3]);
    //Register sigint_handler
    signal(SIGINT, sigint_handler);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        perror("Socket creation Failed!\n");
        return -1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    int res;
    res = hackrf_init();
    if (res != HACKRF_SUCCESS) {
        fprintf(stderr, "Failed to initialize HackRF: %s\n", hackrf_error_name(res));
        return -1;
    }
    res = hackrf_open(&d);
    if (res != HACKRF_SUCCESS) {
        fprintf(stderr, "Failed to open HackRF device: %s\n", hackrf_error_name(res));
        hackrf_exit();
        return -1;
    }
    
    res = hackrf_set_freq(d, CENTER_FREQ);
    if (res != HACKRF_SUCCESS) {
        fprintf(stderr, "Failed to set frequency: %s\n", hackrf_error_name(res));
        hackrf_close(d);
        hackrf_exit();
        return -1;
    }
    res = hackrf_set_sample_rate(d, SAMPLE_RATE);
    if (res != HACKRF_SUCCESS) {
        fprintf(stderr, "Failed to set sample rate: %s\n", hackrf_error_name(res));
        hackrf_close(d);
        hackrf_exit();
        return -1;
    }
    res = hackrf_set_lna_gain(d, LNA_GAIN);
    if (res != HACKRF_SUCCESS) {
        fprintf(stderr, "Failed to set LNA gain: %s\n", hackrf_error_name(res));
        hackrf_close(d);
        hackrf_exit();
        return -1;
    }
    printf("Receiving signals at %f Hz...\n", (float)CENTER_FREQ);
    res = hackrf_start_rx(d, callback, NULL);
    if (res != HACKRF_SUCCESS) {
        fprintf(stderr, "Failed to start RX mode: %s\n", hackrf_error_name(res));
        hackrf_close(d);
        hackrf_exit();
        return -1;
    }
    //sleep(10);
    while(running){
        sleep(0);
    }
    sigint_handler(SIGINT);
    return 0;
}
