
#include "EAR_P2P.h"

Define_Module(EAR_P2P);



void EAR_P2P::handleMessage(cMessage *msg)
{

}


int EAR_P2P::Stitch_long_distance_EAR_P2P(int Source_node,int Destination_node,int Landmark_node)
{
    int output;
    if(Source_node==0 && Destination_node==0 && Landmark_node==0)
    {
        output=1;
    }
    else if(Source_node==0 && Destination_node==0 && Landmark_node==1)
    {
        output=2;
    }
    else if(Source_node==0 && Destination_node==1 && Landmark_node==0)
      {
        output=0;
      }

    else if(Source_node==0 && Destination_node==1 && Landmark_node==1)
      {
        output=0;
      }


    else if(Source_node==1 && Destination_node==0 && Landmark_node==0)
      {
        output=1;
      }


    else if(Source_node==1 && Destination_node==0 && Landmark_node==1)
      {
        output=2;
      }
    else if(Source_node==1 && Destination_node==1 && Landmark_node==0)
      {
        output=0;
      }else
      {
          output=1;
      }


return output;


}

void EAR_P2P::initialize()
{
int res=Agent2_EAR_P2P(100,45,5);
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

int EAR_P2P::Agent2_EAR_P2P(int Channel_capacity,int SNR,int link_lifetime)
{

    int output;

    if(Channel_capacity==0 && SNR==0 && link_lifetime==0)
    {
        output=0;
    }
    else if(Channel_capacity==0 && SNR==0 && link_lifetime==1)
    {
        output=0;
    }
    else if(Channel_capacity==0 && SNR==1 && link_lifetime==0)
      {
        output=0;
      }

    else if(Channel_capacity==0 && SNR==1 && link_lifetime==1)
      {
        output=0;
      }


    else if(Channel_capacity==1 && SNR==0 && link_lifetime==0)
      {
        output=0;
      }


    else if(Channel_capacity==1 && SNR==0 && link_lifetime==1)
      {
        output=1;
      }
    else if(Channel_capacity==1 && SNR==1 && link_lifetime==0)
      {
        output=1;
      }else
      {
          output=2;
      }

return output;

}
int EAR_P2P::Link_Oriented_Metrics(int Deadline ,int Packet_Size,int Mobility)
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

int EAR_P2P::Agent3_EAR_P2P(int val1,int val2,int val3)
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
