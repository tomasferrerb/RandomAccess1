packet MENSAJE
{
    int srcAddr;  // source address
    int whatIs=0;//0 RTS, 1 CTS, 2 DATA, 3 BROADCAST
    int lengthData=0;//bits
    simtime_t backOffTime=0; //back off time, only for RTS
}