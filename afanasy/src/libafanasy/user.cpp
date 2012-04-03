#include "user.h"

#include <stdio.h>
#include <memory.h>

#include "msg.h"
#include "address.h"
#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

User::User( const std::string & username, const std::string & host):
	m_host_name( host),
	m_max_running_tasks(  af::Environment::getMaxRunningTasksNumber() ),
	m_errors_retries(    af::Environment::getTaskErrorRetries() ),
	m_errors_avoid_host( af::Environment::getErrorsAvoidHost()  ),
	m_errors_task_same_host( af::Environment::getTaskErrorsSameHost() ),
	m_errors_forgive_time( af::Environment::getErrorsForgiveTime()),
	m_jobs_life_time( 0),
	m_time_register( 0)
{
	m_name = username;
	m_priority = af::Environment::getPriority();

	construct();
}

User::User( int uid)
{
	m_id = uid;
	construct();
}

User::User( Msg * msg)
{
	read( msg);
}

void User::construct()
{
	m_jobs_num = 0;
	m_running_jobs_num = 0;
	need = 0.0f;
	m_running_tasks_num = 0;
	m_time_online = 0;

	m_hosts_mask.setCaseInsensitive();

	m_hosts_mask_exclude.setCaseInsensitive();
	m_hosts_mask_exclude.setExclude();
}

User::~User()
{
AFINFO("User::~User:")
}

void User::readwrite( Msg * msg)
{
	Node::readwrite( msg);

	rw_uint32_t( m_state,                 msg);
	rw_uint32_t( m_flags,                 msg);
	rw_int64_t ( m_time_register,         msg);
	rw_String  ( m_host_name,             msg);
	rw_int32_t ( m_max_running_tasks,     msg);
	rw_uint8_t ( m_errors_retries,        msg);
	rw_uint8_t ( m_errors_avoid_host,     msg);
	rw_uint8_t ( m_errors_task_same_host, msg);
	rw_int32_t ( m_errors_forgive_time,   msg);
	rw_int64_t ( m_time_online,           msg);
	rw_int32_t ( m_jobs_life_time,        msg);
	rw_int32_t ( m_jobs_num,              msg);
	rw_int32_t ( m_running_jobs_num,      msg);
	rw_int32_t ( m_running_tasks_num,     msg);
	rw_float   ( need,                    msg);
	rw_RegExp  ( m_hosts_mask,            msg);
	rw_RegExp  ( m_hosts_mask_exclude,    msg);
	rw_String  ( m_annotation,            msg);
	rw_String  ( m_customdata,            msg);
}

void User::setPermanent( bool value)
{
   if( value )
   {
      m_state = m_state | Permanent;
      m_time_register = time( NULL);
   }
   else
      m_state = m_state & (~Permanent);
}

void User::setJobsSolveMethod( int i_method )
{
    switch( i_method)
    {
    case af::Node::SolveByOrder:
        m_state = m_state & (~SolveJobsParrallel);
        break;
    case af::Node::SolveByPriority:
        m_state = m_state | SolveJobsParrallel;
        break;
    }
}

int User::calcWeight() const
{
	int weight = Node::calcWeight();
	weight += sizeof(User) - sizeof( Node);
	weight += weigh(m_host_name);
	weight += m_hosts_mask.weigh();
	weight += m_hosts_mask_exclude.weigh();
	weight += weigh(m_annotation);
	return weight;
}

const std::string User::generateErrorsSolvingString() const
{
   std::ostringstream stream;
   generateErrorsSolvingStream( stream);
   return stream.str();
}

void User::generateErrorsSolvingStream( std::ostringstream & stream) const
{
   stream << "E-" << int(m_errors_avoid_host) << "j|"
         << int(m_errors_task_same_host) << "t|"
         << int(m_errors_retries) << "r";
   if( m_errors_forgive_time > 0 ) stream << " F" << af::time2strHMS( m_errors_forgive_time, true);
}

void User::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
      stream << "User name = \"" << m_name << "\" (id=" << m_id << "):";
      stream << "\n Priority = " << int(m_priority);
      stream << "\n    Each point gives 10% bonus";
      stream << "\n Jobs = " << m_jobs_num << " / active jobs = " << m_running_jobs_num;
      if( m_jobs_life_time > 0 ) stream << "\n Jobs life time = " << af::time2strHMS( m_jobs_life_time, true);
      stream << "\n Maximum Running Tasks = " << m_max_running_tasks;
      if( m_max_running_tasks < 1 ) stream << " (no limit)";
      stream << "\n Running Tasks Number = " << m_running_tasks_num;
      if( hasHostsMask())        stream << "\n Hosts Mask = \"" << m_hosts_mask.getPattern() << "\"";
      if( hasHostsMaskExclude()) stream << "\n Exclude Hosts Mask = \"" << m_hosts_mask_exclude.getPattern() << "\"";

      stream << "\n Errors Solving: ";
      generateErrorsSolvingStream( stream);
      stream << "\n    Errors To Avoid Host = " << int(m_errors_avoid_host);
      stream << "\n    Maximum Errors Same Task = " << int(m_errors_task_same_host);
      stream << "\n    Task Errors To Retry = " << int(m_errors_retries);
      stream << "\n    Errors Forgive Time = " << af::time2strHMS( m_errors_forgive_time, true);
      if( m_errors_forgive_time == 0 ) stream << " (infinite, no forgiving)";

      if( m_host_name.size() != 0) stream << "\n Last host = \"" << m_host_name << "\"";
      if( isPermanent())
      {
         stream << "\n User is permanent (stored in database)";
         stream << "\n Registration time = " << time2str( m_time_register);
      }
      else stream << "\n (user is temporal)";
      stream << "\n Online time = " << time2str( m_time_online);
      if( m_annotation.size()) stream << "\n" << m_annotation;
      if( m_customdata.size()) stream << "\nCustom Data:\n" << m_customdata;
      //stream << "\n Memory = " << calcWeight() << " bytes.";
   }
   else
   {
      stream << "#" << m_id << ":" << int(m_priority)
            << " " << m_name
            << " j" << m_jobs_num << "/" << m_running_jobs_num
            << " r" << m_running_tasks_num << "/" << m_max_running_tasks
            << " " << time2str( m_time_online)
            << " " <<  m_host_name
            << " " << (isPermanent() == 1 ? "P" : "T")
            << " - " << calcWeight() << " bytes.";
   }
}
