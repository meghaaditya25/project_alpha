
#include "CAOD.h"

Define_Module(CAOD);



void CAOD::handleMessage(cMessage *msg)
{

}


int CAOD::Node_Oriented_Metrics(int Node_energy,int ETX,int distance)
{
    int output;
    if(Node_energy==0 && ETX==0 && distance==0)
    {
        output=1;
    }
    else if(Node_energy==0 && ETX==0 && distance==1)
    {
        output=2;
    }
    else if(Node_energy==0 && ETX==1 && distance==0)
      {
        output=0;
      }

    else if(Node_energy==0 && ETX==1 && distance==1)
      {
        output=0;
      }


    else if(Node_energy==1 && ETX==0 && distance==0)
      {
        output=1;
      }


    else if(Node_energy==1 && ETX==0 && distance==1)
      {
        output=2;
      }
    else if(Node_energy==1 && ETX==1 && distance==0)
      {
        output=0;
      }else
      {
          output=1;
      }


return output;


}

void CAOD::initialize()
{
int res=Node_Oriented_Metrics(100,45,5);
if(res<2)
{
    cMessage *resmsg=new cMessage();
    handleMessage(resmsg);
}
else
{
    cMessage *resmsg=new cMessage();
    handleMessage(resmsg);
}
}

int CAOD::Channel_Oriented_Metrics(int Channel_capacity,int Channel_Bandwidth,int RSRQ)
{

    int output;

    if(Channel_capacity==0 && Channel_Bandwidth==0 && RSRQ==0)
    {
        output=0;
    }
    else if(Channel_capacity==0 && Channel_Bandwidth==0 && RSRQ==1)
    {
        output=0;
    }
    else if(Channel_capacity==0 && Channel_Bandwidth==1 && RSRQ==0)
      {
        output=0;
      }

    else if(Channel_capacity==0 && Channel_Bandwidth==1 && RSRQ==1)
      {
        output=0;
      }


    else if(Channel_capacity==1 && Channel_Bandwidth==0 && RSRQ==0)
      {
        output=0;
      }


    else if(Channel_capacity==1 && Channel_Bandwidth==0 && RSRQ==1)
      {
        output=1;
      }
    else if(Channel_capacity==1 && Channel_Bandwidth==1 && RSRQ==0)
      {
        output=1;
      }else
      {
          output=2;
      }

return output;

}
int CAOD::Link_Oriented_Metrics(int Deadline ,int Packet_Size,int Mobility)
{



     int output;

     if(Deadline ==0 && Packet_Size==0 && Mobility==0)
     {
         output=0;
     }
     else if(Deadline ==0 && Packet_Size==0 && Mobility==1)
     {
         output=0;
     }
     else if(Deadline ==0 && Packet_Size==1 && Mobility==0)
       {
         output=0;
       }

     else if(Deadline ==0 && Packet_Size==1 && Mobility==1)
       {
         output=2;
       }


     else if(Deadline ==1 && Packet_Size==0 && Mobility==0)
       {
         output=2;
       }


     else if(Deadline ==1 && Packet_Size==0 && Mobility==1)
       {
         output=1;
       }
     else if(Deadline ==1 && Packet_Size==1 && Mobility==0)
       {
         output=2;
       }else
       {
           output=0;
       }

 return output;

}

int CAOD::Outputfuzzy(int val1,int val2,int val3)
{


    int output;

       if(val1==0 && val2==0 && val3==0)
       {
           output=0;
       }
       else if(val1==0 && val2==0 && val3==1)
       {
           output=0;
       }
       else if(val1==0 && val2==1 && val3==0)
         {
           output=0;
         }

       else if(val1==0 && val2==1 && val3==1)
         {
           output=1;
         }


       else if(val1==1 && val2==0 && val3==0)
         {
           output=1;
         }


       else if(val1==1 && val2==0 && val3==1)
         {
           output=2;
         }
       else if(val1==1 && val2==1 && val3==0)
         {
           output=2;
         }else
         {
             output=3;
         }

   return output;


}
