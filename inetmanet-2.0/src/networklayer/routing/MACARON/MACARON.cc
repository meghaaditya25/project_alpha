
#include "MACARON.h"

Define_Module(MACARON);



void MACARON::handleMessage(cMessage *msg)
{

}


int MACARON::Stitch_long_distance_MACARON(int Source_node,int Destination_node,int Landmark_node)
{
    int vicinity_nodes;
    if(Source_node==0 && Destination_node==0 && Landmark_node==0)
    {
        vicinity_nodes=1;
    }
    else if(Source_node==0 && Destination_node==0 && Landmark_node==1)
    {
        vicinity_nodes=2;
    }
    else if(Source_node==0 && Destination_node==1 && Landmark_node==0)
      {
        vicinity_nodes=0;
      }

    else if(Source_node==0 && Destination_node==1 && Landmark_node==1)
      {
        vicinity_nodes=0;
      }


    else if(Source_node==1 && Destination_node==0 && Landmark_node==0)
      {
        vicinity_nodes=1;
      }


    else if(Source_node==1 && Destination_node==0 && Landmark_node==1)
      {
        vicinity_nodes=2;
      }
    else if(Source_node==1 && Destination_node==1 && Landmark_node==0)
      {
        vicinity_nodes=0;
      }else
      {
          vicinity_nodes=1;
      }


return vicinity_nodes;


}

void MACARON::initialize()
{
int res=Agent2_MACARON(100,45,5);
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

int MACARON::Agent2_MACARON(int Channel_capacity,int SNR,int link_lifetime)
{

    int vicinity_nodes;

    if(Channel_capacity==0 && SNR==0 && link_lifetime==0)
    {
        vicinity_nodes=0;
    }
    else if(Channel_capacity==0 && SNR==0 && link_lifetime==1)
    {
        vicinity_nodes=0;
    }
    else if(Channel_capacity==0 && SNR==1 && link_lifetime==0)
      {
        vicinity_nodes=0;
      }

    else if(Channel_capacity==0 && SNR==1 && link_lifetime==1)
      {
        vicinity_nodes=0;
      }


    else if(Channel_capacity==1 && SNR==0 && link_lifetime==0)
      {
        vicinity_nodes=0;
      }


    else if(Channel_capacity==1 && SNR==0 && link_lifetime==1)
      {
        vicinity_nodes=1;
      }
    else if(Channel_capacity==1 && SNR==1 && link_lifetime==0)
      {
        vicinity_nodes=1;
      }else
      {
          vicinity_nodes=2;
      }

return vicinity_nodes;

}
int MACARON::Link_Oriented_Metrics(int Routing ,int OLSR ,int Mobility)
{



     int vicinity_nodes;

     if(Routing ==0 && OLSR ==0 && Mobility==0)
     {
         vicinity_nodes=0;
     }
     else if(Routing ==0 && OLSR ==0 && Mobility==1)
     {
         vicinity_nodes=0;
     }
     else if(Routing ==0 && OLSR ==1 && Mobility==0)
       {
         vicinity_nodes=0;
       }

     else if(Routing ==0 && OLSR ==1 && Mobility==1)
       {
         vicinity_nodes=2;
       }


     else if(Routing ==1 && OLSR ==0 && Mobility==0)
       {
         vicinity_nodes=2;
       }


     else if(Routing ==1 && OLSR ==0 && Mobility==1)
       {
         vicinity_nodes=1;
       }
     else if(Routing ==1 && OLSR ==1 && Mobility==0)
       {
         vicinity_nodes=2;
       }else
       {
           vicinity_nodes=0;
       }

 return vicinity_nodes;

}

int MACARON::Agent3_MACARON(int val1,int val2,int val3)
{


    int vicinity_nodes;

       if(val1==0 && val2==0 && val3==0)
       {
           vicinity_nodes=0;
       }
       else if(val1==0 && val2==0 && val3==1)
       {
           vicinity_nodes=0;
       }
       else if(val1==0 && val2==1 && val3==0)
         {
           vicinity_nodes=0;
         }

       else if(val1==0 && val2==1 && val3==1)
         {
           vicinity_nodes=1;
         }


       else if(val1==1 && val2==0 && val3==0)
         {
           vicinity_nodes=1;
         }


       else if(val1==1 && val2==0 && val3==1)
         {
           vicinity_nodes=2;
         }
       else if(val1==1 && val2==1 && val3==0)
         {
           vicinity_nodes=2;
         }else
         {
             vicinity_nodes=3;
         }

   return vicinity_nodes;


}
