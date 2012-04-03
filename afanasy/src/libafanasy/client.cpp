#include "client.h"

#include <stdio.h>
#include <memory.h>

#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Client::Client( int flags, int Id):
	m_time_launch( 0),
	m_time_register( 0),
	m_time_update( 0)
{
   m_id = Id;
   if( flags & GetEnvironment )
   {
      bool verbose = false;
#ifdef WINNT
//      verbose = true;
#endif //WINNT
#ifdef MACOSX
//      verbose = true;
#endif //MACOSX
	  af::NetIF::getNetIFs( m_netIFs, verbose);

      std::cout << "Network Interfaces:" << std::endl;
	  for( int i = 0; i < m_netIFs.size(); i++)
      {
         std::cout << "   ";
		 m_netIFs[i]->stdOut(true);
      }

	  m_address.setPort( af::Environment::getClientPort());
	  m_time_launch = time(NULL);
	  m_user_name = af::Environment::getUserName();
	  m_name = af::Environment::getHostName();
	  m_version = af::Environment::getVersionCGRU();
   }
}

Client::~Client()
{
   clearNetIFs();
}

void Client::clearNetIFs()
{
   for( int i = 0; i < m_netIFs.size(); i++) if( m_netIFs[i]) delete m_netIFs[i];
   m_netIFs.clear();
}

void Client::grabNetIFs( std::vector<NetIF*> & otherNetIFs)
{
   clearNetIFs();
   for( int i = 0; i < otherNetIFs.size(); i++)
   {
	  m_netIFs.push_back( otherNetIFs[i]);
      otherNetIFs[i] = NULL;
   }
}

void Client::setRegisterTime()
{
   m_time_register = time( NULL);
   m_time_update = m_time_register;
}

int Client::calcWeight() const
{
   int weight = Node::calcWeight();
//printf("Client::calcWeight: Node::calcWeight: %d bytes\n", weight);
   weight += sizeof(Client) - sizeof( Node);
   weight += weigh( m_user_name);
   weight += weigh( m_version );
   weight += m_address.calcWeight();
   for( int i = 0; i < m_netIFs.size(); i++) weight += m_netIFs[i]->calcWeight();
//printf("Client::calcWeight: %d bytes ( sizeof Client = %d)\n", weight, sizeof( Client));
   return weight;
}
