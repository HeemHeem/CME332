/*************************************************************************
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
* All rights reserved. All use of this software and documentation is     *
* subject to the License Agreement located at the end of this file below.*
**************************************************************************
* Description:                                                           *
* The following is a simple hello world program running MicroC/OS-II.The * 
* purpose of the design is to be a very simple application that just     *
* demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
* for issues such as checking system call return codes. etc.             *
*                                                                        *
* Requirements:                                                          *
*   -Supported Example Hardware Platforms                                *
*     Standard                                                           *
*     Full Featured                                                      *
*     Low Cost                                                           *
*   -Supported Development Boards                                        *
*     Nios II Development Board, Stratix II Edition                      *
*     Nios Development Board, Stratix Professional Edition               *
*     Nios Development Board, Stratix Edition                            *
*     Nios Development Board, Cyclone Edition                            *
*   -System Library Settings                                             *
*     RTOS Type - MicroC/OS-II                                           *
*     Periodic System Timer                                              *
*   -Know Issues                                                         *
*     If this design is run on the ISS, terminal output will take several*
*     minutes per iteration.                                             *
**************************************************************************/


#include <stdio.h>
#include "includes.h"


//#include "includes.h"

#define TASK_STK_SIZE   1024



OS_STK  TestTaskStkA[TASK_STK_SIZE];

OS_STK  TestTaskStkB[TASK_STK_SIZE];

OS_STK  TestTaskStkC[TASK_STK_SIZE];

OS_STK  TestTaskStkD[TASK_STK_SIZE];



void TestTaskA(void *pdata);

void TestTaskB(void *pdata);

void TestTaskC(void *pdata);

void TestTaskD(void *pdata);



int main(void)

{

    //OSInit();



    OSTaskCreate(TestTaskA, (void *)0, &TestTaskStkA[TASK_STK_SIZE-1], 1);



    OSStart();



    return 0;

}



void TestTaskA(void *pdata)

{

    printf("A0\n");



    OSTaskCreate(TestTaskB, (void *)0, &TestTaskStkB[TASK_STK_SIZE-1], 4);

    OSTaskCreate(TestTaskC, (void *)0, &TestTaskStkC[TASK_STK_SIZE-1], 3);

    OSTaskCreate(TestTaskD, (void *)0, &TestTaskStkD[TASK_STK_SIZE-1], 2);



    while (1)

    {   printf("A1\n");

        OSTaskSuspend(OS_PRIO_SELF);

    }

}



void TestTaskB(void *pdata)

{

    printf("B0\n");

    while (1)

    {   printf("B1\n");

        OSTimeDly(1);

    }

}



void TestTaskC(void *pdata)

{   while (1)

    {

        printf("C1\n");

        OSTimeDly(2);

    }

}



void TestTaskD(void *pdata)

{   while (1)

    {

        printf("D1\n");

        OSTimeDly(3);

        OSTaskResume(1);

    }

}
/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
