#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_PATIENT 30
#define MAX_ROOM 8
#define UNIT_CAPACITY 3

void randwait(void);
int find_unit_id(void);
int number_of_patient_in_unit(int index);

void *patient(void *num);
void *room(void *num2);

sem_t test_unit_sem[MAX_ROOM];
sem_t vaccinate[MAX_ROOM];
sem_t mutex[MAX_ROOM];

int all_done = 0;
int room_control_counter = MAX_PATIENT / 2;
int counter = MAX_PATIENT;

int main(void)
{
    pthread_t roomtid[MAX_ROOM];
    pthread_t patientid[MAX_PATIENT];

    int i;
    int j;
    int patient_numbers[MAX_PATIENT];
    int room_numbers[MAX_ROOM];

    srand((unsigned int)time(NULL));

    for (i = 0; i < MAX_PATIENT; i++) {
        patient_numbers[i] = i + 1;
    }

    for (j = 0; j < MAX_ROOM; j++) {
        room_numbers[j] = j + 1;
    }

    for (i = 0; i < MAX_ROOM; i++) {
        sem_init(&vaccinate[i], 0, 0);
        sem_init(&test_unit_sem[i], 0, UNIT_CAPACITY);
        sem_init(&mutex[i], 0, 1);
    }

    for (j = 0; j < MAX_ROOM; j++) {
        pthread_create(&roomtid[j], NULL, room, (void *)&room_numbers[j]);
    }

    for (i = 0; i < MAX_PATIENT; i++) {
        randwait();
        pthread_create(&patientid[i], NULL, patient, (void *)&patient_numbers[i]);
    }

    for (i = 0; i < MAX_PATIENT; i++) {
        pthread_join(patientid[i], NULL);
    }

    all_done = 1;

    for (j = 0; j < MAX_ROOM; j++) {
        pthread_join(roomtid[j], NULL);
    }

    for (i = 0; i < MAX_ROOM; i++) {
        sem_destroy(&vaccinate[i]);
        sem_destroy(&test_unit_sem[i]);
        sem_destroy(&mutex[i]);
    }

    printf("All of patients have vaccinated and DEU Hospital has been closed\n");

    return 0;
}

void *patient(void *number)
{
    int num = *(int *)number;
    int numberofpatient;
    int roomid_index = find_unit_id();


    sem_wait(&mutex[roomid_index]);

    sem_post(&mutex[roomid_index]);

    sem_wait(&test_unit_sem[roomid_index]);

    numberofpatient = number_of_patient_in_unit(roomid_index);

    printf("Patient %d arrived at the hospital .. \n", num);
    printf("Patient %d is entering  Covid-19 Test Unit %d \n", num, roomid_index + 1);

    if (numberofpatient == 2) {
        printf("Last 2 people in Test Unit %d.\n", roomid_index + 1);
        printf("Test Unit %d,\n", roomid_index + 1);
        printf("[ X ] , [ ] , [ ] \n");
    } else if (numberofpatient == 1) {
        printf("Last 1 people in Test Unit %d.\n", roomid_index + 1);
        printf("Test Unit %d,\n", roomid_index + 1);
        printf("[ X ] , [ X ] , [ ] \n");
    } else if (numberofpatient == 0) {
        sem_post(&vaccinate[roomid_index]);
        printf("Test Unit %d is full.\n", roomid_index + 1);
        printf("[ X ] , [ X ] , [ X ] \n");
    }

    return NULL;
}

void *room(void *number2)
{
    int num2 = *(int *)number2;
    int i;
    int value;
    int room_index = num2 - 1;

    printf("Test Unit %d is ventilating.\n", num2);

    while (!all_done && counter != 0) {
        if (!all_done || counter == 0) {
            if (counter == 0) {
                break;
            }

            sem_getvalue(&vaccinate[room_index], &value);
            while (value == 0 && counter != 0) {
                sem_getvalue(&vaccinate[room_index], &value);
            }

            if (value == 1) {
                sem_wait(&vaccinate[room_index]);
                printf("Start vaccinating in Test Unit %d.\n", num2);
                randwait();
            }

            if (counter == 0) {
                break;
            }

            printf("Test Unit %d is full.\n", num2);
            sleep(1);

            sem_wait(&mutex[room_index]);

            for (i = 0; i < UNIT_CAPACITY; i++) {
                sem_post(&test_unit_sem[room_index]);
                counter = counter - 1;
            }

            printf("Test Unit %d is ventilating.\n", num2);
            randwait();

            sem_post(&mutex[room_index]);
            printf("Test Unit %d is empty.\n", num2);
        }
    }

    printf("Test Unit %d is closed..\n", num2);
    return NULL;
}

void randwait(void)
{
    int random = rand() % 3 + 1;
    sleep(random);
}

int find_unit_id(void)
{
    int i;
    int min_room = 3;
    int index = 0;
    int value;

    if (room_control_counter != 0) {
        index = rand() % MAX_ROOM;
        room_control_counter = room_control_counter - 1;
    } else {
        for (i = 0; i < MAX_ROOM; i++) {
            sem_getvalue(&test_unit_sem[i], &value);
            if (value <= min_room && value != 0) {
                min_room = value;
                index = i;
            }
        }
    }

    return index;
}

int number_of_patient_in_unit(int index)
{
    int value;

    sem_getvalue(&test_unit_sem[index], &value);

    return value;
}
