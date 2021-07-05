#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> 

///////////////////////////// Definig Constanst
#define MAX_PATIENT 30
#define MAX_ROOM 9

/////////////////////// Necessary Functions 
void randwait();
int FindUnitID();
int NumberOfPatientInUnit(int index);  



/////////////////////////// Thread Function 
void *patient(void *num);
void *room(void *num2);

///////////////////////////// Semaphoresss 
sem_t test_unit_sem[MAX_ROOM];
sem_t vaccinate[MAX_ROOM];
sem_t mutex[MAX_ROOM];

int allDONE = 0;

int oda_kontrol_counter = MAX_PATIENT/2;

int counter = MAX_PATIENT;

int main(int argc, char *argv[]){

    pthread_t roomtid[MAX_ROOM];
    pthread_t patientid[MAX_PATIENT];

    int i ;
    int j;
    int Number[MAX_PATIENT];
    int Number2[MAX_ROOM];

    for ( i = 1; i <= MAX_PATIENT; i++)
    {
        Number[i] = i;
    }
    for ( j = 1; j <MAX_ROOM; j++)
    {
        Number2[j]= j;
    }

        ///////////////////////////////////// Semaphore Inıtiliaze
    for ( i = 1; i < MAX_ROOM; i++)
    {
       sem_init( &vaccinate[i], 0, 0);
       sem_init( &test_unit_sem[i], 0, 3);
       sem_init( &mutex[i], 0, 1);
    }
  

    
    /////////////////////////////////////// Thread Creationsss
    for ( j = 1; j < MAX_ROOM; j++)
    {
        pthread_create(&roomtid[j],NULL,room, (void *)&Number2[j]);
    }
    
    for ( i = 1; i <= MAX_PATIENT; i++)
    {
        randwait();
        pthread_create(&patientid[i],NULL,patient, (void *)&Number[i]);
        
    }

        //  Join the patient thread .
    for ( i = 1; i <= MAX_PATIENT; i++)
    {
        pthread_join(patientid[i],NULL);
       
    }


    
    allDONE = 1;
    
    

    //// Join the room thread .

    for ( j = 1; j < MAX_ROOM; j++)
    {
         pthread_join(roomtid[j],NULL);
         
    }
    
   
    printf("All of patients have vaccinated and DEU Hospital has been closed\n");

    return 0;
}

void *patient(void *number){
    int num = *(int *)number;
    int numberofpatient;
    int roomid_index = FindUnitID();


    sem_wait(&mutex[roomid_index]); ///////////////////////// Begin to critical section .
    
    sem_post(&mutex[roomid_index]);

    sem_wait(&test_unit_sem[roomid_index]); //////// Patient entering test unit .

    numberofpatient = NumberOfPatientInUnit(roomid_index);

    printf("Patient %d arrived at the hospital .. \n", num);
    printf("Patient %d is entering  Covid-19 Test Unit %d \n",num,roomid_index);

   ////////////////////////////////////// Unit state 

    if (numberofpatient == 2)
    {
        printf("Last 2 people in Test Unit %d.\n",roomid_index);
        printf("Test Unit %d,\n",roomid_index);
        printf("[ X ] , [ ] , [ ] \n");
    }
    else if (numberofpatient == 1)
    {
        printf("Last 1 people in Test Unit %d.\n",roomid_index);
        printf("Test Unit %d,\n",roomid_index);
        printf("[ X ] , [ X ] , [ ] \n");
    }
    else if (numberofpatient == 0)
    {
        sem_post(&vaccinate[roomid_index]);
        printf("Test Unit %d is full.\n",roomid_index); /////// Lock the semaphore room .
        printf("[ X ] , [ X ] , [ X ] \n");
    }
}

void *room(void *number2){
    
    
    int num2 = *(int *)number2;
    int i;
   
    int value;
    printf("Test Unit %d is vantilating.\n",num2);
    
    while (!allDONE && counter!=0)
    {
        if (!allDONE || counter == 0)
       {
        if (counter == 0)
       {
           break;
       }

       sem_getvalue(&vaccinate[num2],&value);
       while (value==0 && counter!=0)
       {
           sem_getvalue(&vaccinate[num2],&value);
           
       }
       
         if (value ==1)
         {
              sem_wait(&vaccinate[num2]);//////////////////////////////////// CRITICAL SECTION IS STARTED ....... 
              printf("Start vaccinating in Test Unit %d.\n",num2);
              randwait(5);
              
         }
         if (counter == 0)
         {
             break;
         }
         
           
            printf("Test Unit %d is full.\n",num2);
            sleep(1);
           
            sem_wait(&mutex[num2]);
            
            
            for ( i = 1; i <= 3; i++)
            {
                sem_post(&test_unit_sem[num2]); ///////// Patient is leaving the critical sectionn ...
                counter = counter -1;
            }
            printf("Test Unit %d is vantilating.\n",num2);
            randwait();

            sem_post(&mutex[num2]); ///// Open the test unit ..
            printf("Test Unit %d is empty.\n",num2);    
            
      }
       
    }   

    printf("Test Unit %d is closed..\n",num2); 
}

void randwait() {
     int random = rand()%3+1;
     sleep(random);
}

int FindUnitID(){ //////// Find to available room id . 
    int i;
    int min_room = 3;
    int index ;
    int value;
    
    if (oda_kontrol_counter != 0)
    {
        index =rand()%8+1;
        oda_kontrol_counter = oda_kontrol_counter - 1 ;
    }
    else
    {
    
        for ( i = 1; i < MAX_ROOM; i++)
        {
            sem_getvalue(&test_unit_sem[i],&value);
            if(value <= min_room && value != 0)
            {
            min_room = value;
            index = i;
            }
        } 
    }
    return index;
}

int NumberOfPatientInUnit(int index){ ////////////Find to the number of test unit ....
    int value ;
    

    sem_getvalue(&test_unit_sem[index],&value);

    return value;
}