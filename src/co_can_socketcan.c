/******************************************************************************
   Copyright 2020 Embedded Office GmbH & Co. KG

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
******************************************************************************/

/******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include "co_can_socketcan.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

/******************************************************************************
 * PRIVATE DEFINES
 ******************************************************************************/

static int sock;
static struct sockaddr_can addr;

/******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

static void DrvCanInit(void);
static void DrvCanEnable(uint32_t baudrate);
static int16_t DrvCanSend(CO_IF_FRM *frm);
static int16_t DrvCanRead(CO_IF_FRM *frm);
static void DrvCanReset(void);
static void DrvCanClose(void);

/******************************************************************************
 * PUBLIC VARIABLE
 ******************************************************************************/

/* TODO: rename the variable to match the naming convention:
 *   <device>CanDriver
 */
const CO_IF_CAN_DRV SocketCanDriver = {
    DrvCanInit,
    DrvCanEnable,
    DrvCanRead,
    DrvCanSend,
    DrvCanReset,
    DrvCanClose};

/******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

static void DrvCanInit(void)
{
    struct ifreq ifr;

    printf("[co-socketcan] create socket on interface %s \n", SOCKETCAN_IF_NAME);

    if ((sock = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("[co-socketcan] unable to create socket");
    }

    strcpy(ifr.ifr_name, SOCKETCAN_IF_NAME);
    ioctl(sock, SIOCGIFINDEX, &ifr);

    memset(&addr, 0, sizeof(addr));

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
}

static void DrvCanEnable(uint32_t baudrate)
{
    (void)baudrate;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("[co-socketcan] unable to bind socket");
    }
}

static int16_t DrvCanSend(CO_IF_FRM *frm)
{
    struct can_frame frame = {0};

    frame.can_id = frm->Identifier;
    frame.can_dlc = frm->DLC;
    memcpy(&frame.data[0], &frm->Data[0], frm->DLC);

    if (frm->Identifier > 0x7ff)
    {
        frame.can_id |= CAN_EFF_FLAG;
    }

    if (write(sock, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
    {
        perror("[co-socketcan] send");
        return (-1);
    }

    return (sizeof(CO_IF_FRM));
}

static int16_t DrvCanRead(CO_IF_FRM *frm)
{
    struct can_frame frame;

    if (read(sock, &frame, sizeof(struct can_frame)) < 0)
    {
        perror("[co-socketcan] read");
        return (-1);
    }

    frm->Identifier = frame.can_id & CAN_EFF_MASK;
    frm->DLC = frame.can_dlc;
    memcpy(&frm->Data[0], &frame.data[0], frame.can_dlc);

    return (sizeof(CO_IF_FRM));
}

static void DrvCanReset(void)
{
    /* TODO: reset CAN controller while keeping baudrate */
}

static void DrvCanClose(void)
{
    if (close(sock) < 0)
    {
        perror("[co-socketcan] unable to close socket");
    }
}
