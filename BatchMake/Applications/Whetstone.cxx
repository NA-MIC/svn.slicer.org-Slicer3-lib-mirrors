#include "Whetstone.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#endif  // defined(WIN32) || defined(_WIN32)

/** Constructor */
WhetstoneClock::WhetstoneClock():m_Frequency(1)
{
#if defined(WIN32) || defined(_WIN32)

  LARGE_INTEGER frequency;
  ::QueryPerformanceFrequency(&frequency);

  this->m_Frequency = 
    static_cast< FrequencyType >( (__int64)frequency.QuadPart );

  SYSTEMTIME st1;
  SYSTEMTIME st2;
  FILETIME ft1;
  FILETIME ft2;

  ::memset( &st1, 0, sizeof( st1 ) );
  ::memset( &st2, 0, sizeof( st2 ) );

  st1.wYear = 1601;
  st1.wMonth = 1;
  st1.wDay = 1;

  st2.wYear = 1970;
  st2.wMonth = 1;
  st2.wDay = 1;

  ::SystemTimeToFileTime(&st1, &ft1);
  ::SystemTimeToFileTime(&st2, &ft2);

  LARGE_INTEGER ui1;
  LARGE_INTEGER ui2;

  memcpy( &ui1, &ft1, sizeof( ui1 ) );
  memcpy( &ui2, &ft2, sizeof( ui2 ) );
  
  this->m_Difference = 
    static_cast< TimeStampType >( ui2.QuadPart - ui1.QuadPart) / 
    static_cast< TimeStampType >( 1e7 );

  FILETIME currentTime;
  LARGE_INTEGER intTime;
  LARGE_INTEGER tick;

  ::GetSystemTimeAsFileTime( &currentTime );
  ::QueryPerformanceCounter( &tick );

  memcpy( &intTime, &currentTime, sizeof( intTime ) );

  this->m_Origin = 
    static_cast< TimeStampType >( intTime.QuadPart ) / 
    static_cast< TimeStampType >( 1e7 );

  this->m_Origin -= 
    static_cast< TimeStampType >( (__int64)tick.QuadPart ) / 
    this->m_Frequency;
    
  this->m_Origin +=  this->m_Difference;


#else

  this->m_Frequency = 1e6;;

#endif  // defined(WIN32) || defined(_WIN32)
}

/** Destructor */
WhetstoneClock::~WhetstoneClock()
{
}


/** Returns a timestamp in seconds */
WhetstoneClock::TimeStampType
WhetstoneClock::GetTimeStamp() const
{
#if defined(WIN32) || defined(_WIN32)
  LARGE_INTEGER tick;
  
  ::QueryPerformanceCounter( &tick );

  TimeStampType value = 
      static_cast< TimeStampType >( (__int64)tick.QuadPart ) / 
      this->m_Frequency;

  value += this->m_Origin;

  return value;

#else

  struct timeval tval;

  ::gettimeofday( &tval, 0 );

  TimeStampType value = 
    static_cast< TimeStampType >( tval.tv_sec ) +
    static_cast< TimeStampType >( tval.tv_usec ) / this->m_Frequency;

  return value;

#endif  // defined(WIN32) || defined(_WIN32)
}

/** Returns frequency - clocks per second */
WhetstoneClock::TimeStampType
WhetstoneClock::GetFrequency() const
{
  return m_Frequency;
}

Whetstone::
Whetstone()
{
  m_DesiredCalibrationSeconds = 10;
  m_DesiredBenchmarkSeconds = 100;

  m_TimeUsed = 0;
}

Whetstone::
~Whetstone()
{
}

void Whetstone::
SetDesiredDurations( double calibrationSeconds,
                     double benchmarkSeconds )
{
  m_DesiredCalibrationSeconds = calibrationSeconds;
  m_DesiredBenchmarkSeconds = benchmarkSeconds;
}

void Whetstone::
SetDesiredCalibrationDuration( double calibrationSeconds )
{
  m_DesiredCalibrationSeconds = calibrationSeconds;
}

void Whetstone::
SetDesiredBenchmarkDuration( double benchmarkSeconds )
{
  m_DesiredBenchmarkSeconds = benchmarkSeconds;
}

double Whetstone::
GetMIPS( void )
{
  return 5 * (m_LoopMOPS[4]+m_LoopMOPS[3]+m_LoopMOPS[7])/3;
}

double Whetstone::
GetMFLOPS( void )
{
  return (m_LoopMFLOPS[1]+m_LoopMFLOPS[2]+m_LoopMFLOPS[6])/3;
}

double Whetstone::
GetMWIPS( void )
{
  return m_MWIPS;
}

double Whetstone::
GetMCosOPS( void )
{
  return m_LoopMOPS[5];
}

double Whetstone::
GetMExpOPS( void )
{
  return m_LoopMOPS[8];
}

double Whetstone::
GetFixptOPS( void )
{
  return m_LoopMOPS[4];
}

double Whetstone::
GetIfOPS( void )
{
  return m_LoopMOPS[3];
}

double Whetstone::
GetEqualOPS( void )
{
  return m_LoopMOPS[7];
}

void Whetstone::
ComputeBenchmark( void )
{
  int count = 10;
  long xtra = 1;

  bool calibrate = false;
  m_TimeUsed = 0;
  do
    {
    RunWhetstone(xtra, m_DesiredCalibrationSeconds, calibrate);

    calibrate++;
    count--;

    if (m_TimeUsed > 2.0)
      {
      count = 0;
      }
    else
      {
      xtra = xtra * 5;
      }
    }
  while (count > 0);

  if (m_TimeUsed > 0)
    {
    xtra = (long)((double)(m_DesiredBenchmarkSeconds * xtra) / m_TimeUsed);
    }
  if (xtra < 1) 
    {
    xtra = 1;
    }

  calibrate = false;
  m_TimeUsed = 0;
  RunWhetstone(xtra, m_DesiredCalibrationSeconds, calibrate);

  if (m_TimeUsed>0)
    {
    m_MWIPS = (double)(xtra) * (double)(m_DesiredCalibrationSeconds) 
                             / (10.0 * m_TimeUsed);
    }
  else
    {
    m_MWIPS = 0;
    }  

  if ( m_Check == 0 )
    {
    std::cerr << "Warning: inconsistent results." << std::endl;
    }
}

void Whetstone::
RunWhetstone(long xtra, double desiredTime, bool calibrate)
{

  long n1,n2,n3,n4,n5,n6,n7,n8,i,ix,n1mult;
  RealType x,y,z;              
  long j,k,l;
  RealType e1[4];
  double timea,timeb;

  RealType t =  0.49999975;
  RealType t0 = t;        
  RealType t1 = 0.50000025;
  RealType t2 = 2.0;

  m_Check = 0.0;

  n1 = static_cast<long int>(12*desiredTime);
  n2 = static_cast<long int>(14*desiredTime);
  n3 = static_cast<long int>(345*desiredTime);
  n4 = static_cast<long int>(210*desiredTime);
  n5 = static_cast<long int>(32*desiredTime);
  n6 = static_cast<long int>(899*desiredTime);
  n7 = static_cast<long int>(616*desiredTime);
  n8 = static_cast<long int>(93*desiredTime);
  n1mult = 10;

  /* Section 1, Array elements */

  e1[0] = 1.0;
  e1[1] = -1.0;
  e1[2] = -1.0;
  e1[3] = -1.0;
  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      {
      for(i=0; i<n1*n1mult; i++)
        {
        e1[0] = (e1[0] + e1[1] + e1[2] - e1[3]) * t;
        e1[1] = (e1[0] + e1[1] - e1[2] + e1[3]) * t;
        e1[2] = (e1[0] - e1[1] + e1[2] + e1[3]) * t;
        e1[3] = (-e1[0] + e1[1] + e1[2] + e1[3]) * t;
        }
      t = 1.0 - t;
      }
    t =  t0;                    
    }
  timeb = (m_Clock.GetTimeStamp()-timea)/(double)(n1mult);
  UpdateTime((double)(n1*16)*(double)(xtra), 1, e1[3],
             timeb, calibrate, 1);

  /* Section 2, Array as parameter */

  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      { 
      for(i=0; i<n2; i++)
        {
        Pa(e1,t,t2);
        }
      t = 1.0 - t;
      }
    t =  t0;
    }
  timeb = m_Clock.GetTimeStamp()-timea;
  UpdateTime((double)(n2*96)*(double)(xtra), 1, e1[3],
             timeb, calibrate, 2);

  /* Section 3, Conditional jumps */
  j = 1;
  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      {
      for(i=0; i<n3; i++)
        {
        if(j==1)       j = 2;
        else           j = 3;
        if(j>2)        j = 0;
        else           j = 1;
        if(j<1)        j = 1;
        else           j = 0;
        }
      }
    }
  timeb = m_Clock.GetTimeStamp()-timea;
  UpdateTime((double)(n3*3)*(double)(xtra), 2, (double)(j),
             timeb, calibrate, 3);

  /* Section 4, Integer arithmetic */
  j = 1;
  k = 2;
  l = 3;
  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      {
      for(i=0; i<n4; i++)
        {
        j = j *(k-j)*(l-k);
        k = l * k - (l-j) * k;
        l = (l-k) * (k+j);
        e1[l-2] = j + k + l;
        e1[k-2] = j * k * l;
        }
      }
    }
  timeb = m_Clock.GetTimeStamp()-timea;
  x = e1[0]+e1[1];
  UpdateTime((double)(n4*15)*(double)(xtra), 2, x,
             timeb, calibrate, 4);

  /* Section 5, Trig functions */
  x = 0.5;
  y = 0.5;
  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      {
      for(i=1; i<n5; i++)
        {
        x = t*atan(t2*sin(x)*cos(x)/(cos(x+y)+cos(x-y)-1.0));
        y = t*atan(t2*sin(y)*cos(y)/(cos(x+y)+cos(x-y)-1.0));
        }
      t = 1.0 - t;
      }
    t = t0;
    }
  timeb = m_Clock.GetTimeStamp()-timea;
  UpdateTime((double)(n5*26)*(double)(xtra), 2, y,
             timeb, calibrate, 5);

  /* Section 6, Procedure calls */
  x = 1.0;
  y = 1.0;
  z = 1.0;
  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      {
      for(i=0; i<n6; i++)
        {
        P3(&x,&y,&z,t,t1,t2);
        }
      }
    }
  timeb = m_Clock.GetTimeStamp()-timea;
  UpdateTime((double)(n6*6)*(double)(xtra), 1, z,
             timeb, calibrate, 6);

  /* Section 7, Array refrences */
  j = 0;
  k = 1;
  l = 2;
  e1[0] = 1.0;
  e1[1] = 2.0;
  e1[2] = 3.0;
  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      {
      for(i=0;i<n7;i++)
        {
        Po(e1,j,k,l);
        }
      }
    }
  timeb = m_Clock.GetTimeStamp()-timea;
  UpdateTime((double)(n7*3)*(double)(xtra), 2, e1[2],
             timeb, calibrate, 7);

  /* Section 8, Standard functions */
  x = 0.75;
  timea = m_Clock.GetTimeStamp();
    {
    for (ix=0; ix<xtra; ix++)
      {
      for(i=0; i<n8; i++)
        {
        x = sqrt(exp(log(x)/t1));
        }
      }
    }
  timeb = m_Clock.GetTimeStamp()-timea;
  UpdateTime((double)(n8*4)*(double)(xtra), 2, x,
             timeb, calibrate, 8);

  return;
}


void Whetstone::
Pa(RealType e[4], RealType t, RealType t2)
{
  long j;
  for(j=0;j<6;j++)
    {
    e[0] = (e[0]+e[1]+e[2]-e[3])*t;
    e[1] = (e[0]+e[1]-e[2]+e[3])*t;
    e[2] = (e[0]-e[1]+e[2]+e[3])*t;
    e[3] = (-e[0]+e[1]+e[2]+e[3])/t2;
    }

  return;
}

void Whetstone::
Po(RealType e1[4], long j, long k, long l)
{
  e1[j] = e1[k];
  e1[k] = e1[l];
  e1[l] = e1[j];
  return;
}

void Whetstone::
P3(RealType *x, RealType *y, RealType *z,
   RealType t, RealType t1, RealType t2)
{
  *x = *y;
  *y = *z;
  *x = t * (*x + *y);
  *y = t1 * (*x + *y);
  *z = (*x + *y)/t2;
  return;
}


void Whetstone::
UpdateTime(double ops, int type, double checknum,
       double time, bool calibrate, int section)
{
  double mops, mflops;

  m_Check = m_Check + checknum;
  m_LoopTime[section] = time;
  m_TimeUsed =  m_TimeUsed + time;

  if (!calibrate)
    {              
    if (type == 1)
      {
      if (time>0)
        {
        mflops = ops/(1000000L*time);
        }
      else
        {
        mflops = 0;
        }
      m_LoopMOPS[section] = 99999;
      m_LoopMFLOPS[section] = mflops;
      }
    else
      {
      if (time>0)
        {
        mops = ops/(1000000L*time);
        }
      else
        {
        mops = 0;
        }
      m_LoopMOPS[section] = mops;
      m_LoopMFLOPS[section] = 0;                 
      }
    }

  return;
}

