/*
 * Project 2
 * Dr. Lu Su
 * Writers: Sam Gura and Dakota Lester
 * Date: 4/7/2018
 * File: gbn.cpp
 * I have read and understood the course academic integrity policy
 */
#include "../include/simulator.h"
#include <iostream>
#include <string.h>
#include <vector>
/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer 
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#define RTT 25.0
#define CHUNKSIZE 20
#define BUFFER 1000
#define AHOST 0
#define BHOST 1

using namespace std;

int ackflag, aseq, bseq;
int check = 0;
int seq = 0;
int lastsucess = 0;
int numready = 0;
int lastsequence = 0;
vector<pkt> pkts;

struct pkt last;

// The packet must be created first
// followed by the checksum
// This is to avoid scoping issues
int checksum(struct pkt);

struct pkt *createPacket(struct msg message)
{
    struct pkt *packet = new struct pkt;
    (*packet).seqnum = seq;
    (*packet).acknum = seq;
    strcpy((*packet).payload, message.data);
    (*packet).checksum = checksum((*packet));
    return packet;
}

int checksum(struct pkt packet)
{
    char data[CHUNKSIZE];
    strcpy(data, packet.payload);
    int localchecksum = 0;
    int i = 0;
    while(i < CHUNKSIZE && data[i] != '\0')
    {
        localchecksum += data[i];
        i++;
    }

    localchecksum += packet.seqnum;
    localchecksum += packet.acknum;

    return localchecksum;   
}
/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    cout << "Running A_Output..." << endl;
    pkts.push_back(*createPacket(message));
    if(numready == 0)
    {
        last = pkts.at(seq);
        tolayer3(AHOST, last);
        cout << seq << endl;
        seq++;
		starttimer(AHOST, RTT);
		cout << "A_Output timer start" << endl;
		numready++;
    }
    else if(numready < getwinsize())
    {
        last = pkts.at(seq);
        tolayer3(AHOST, last);
        seq = seq + 1;
    	numready++;
    }
    
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
	cout << "Running A_input..." << endl;
    //cout << "A Sequence value: " << aseq << endl;
    //cout << "Packet acknum: " << packet.acknum << endl;
    ackflag = 1;
    //if(packet.acknum == aseq)
    //{
    if(packet.acknum == lastsequence + 1)
    {
    	lastsequence++;
        cout << "Packet acknum: " << packet.acknum << endl; 
    }
	else if(packet.acknum == lastsucess + getwinsize())
	{
        cout << "BALH" << endl;
        cout << endl;
        lastsucess += getwinsize();
        stoptimer(AHOST);
        numready = 0;
	}
    //}
    //aseq = packet.acknum;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    cout << "Running A_timerinterrupt..." << endl;
    cout << "numready: " << numready << endl;
    for (int i = lastsequence; i < lastsequence + getwinsize() && i < numready; i++)
    {
        last = pkts.at(i);
        tolayer3(AHOST, last);
    }
    starttimer(AHOST, RTT);
}  

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    //cout << "Running A_init..." << endl;
    ackflag = 1;
    aseq = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    cout << "Running B_Input..." << endl;
    cout << "B INPUT PACKET: " <<packet.acknum << endl;
    //int bleh = checksum(packet);
    //cout << "Checksum from B_Input: " << bleh << endl;
    if(bseq == packet.seqnum && checksum(packet) == packet.checksum)
    {
        cout << "Worked for both equal" << endl;
        tolayer5(BHOST, packet.payload);
        pkt *ACK = new struct pkt;
        (*ACK).acknum = bseq;
        (*ACK).checksum = packet.seqnum;
        tolayer3(BHOST, *ACK);
        //cout << "ACK Checksum: " << checksum(packet) << endl;
        bseq++;
    }
    else if(bseq != packet.seqnum && checksum(packet) == packet.checksum)
    {
        pkt *ACK = new struct pkt;
        (*ACK).acknum = bseq - 1;
        (*ACK).checksum = packet.seqnum;
        tolayer3(BHOST, *ACK);
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    //cout << "Running B_init..." << endl;
    bseq = 0;
}

