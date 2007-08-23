/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: Timer.cxx,v $
  Language:  C++
  Date:      $Date: 2004/11/30 20:04:23 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifdef DARWIN_SPECIFIC
#include <sys/time.h>
#endif

#include "Timer.h"

Timer
::Timer()
{
  reset();
}

void
Timer
::start()
{
  if (!_isRunning)
    {
      _ftime(_lastStartTime);
      _isRunning  = true;
    }  
}

void
Timer
::stop()
{
  if (_isRunning)
    {
      timeb stopTime;
      _ftime(stopTime);
      
      _totalStartToStopSeconds += 
  static_cast<unsigned long>(stopTime.time - _lastStartTime.time);

      _additionalTotalStartToStopMilliseconds += 
  static_cast<long>(stopTime.millitm) - 
  static_cast<long>(_lastStartTime.millitm);

      _isRunning = false;
    }
}

void
Timer
::reset()
{
  _totalStartToStopSeconds = 0;
  _additionalTotalStartToStopMilliseconds = 0;
  _isRunning = false;
}

unsigned long
Timer
::getSeconds() const
{
  if (_isRunning)
    {
      timeb now; 
      _ftime(now);
      return _totalStartToStopSeconds + 
  static_cast<unsigned long>(now.time - _lastStartTime.time);
    }
  else
    {
      return _totalStartToStopSeconds;
    }
}

unsigned long
Timer
::getMilliseconds() const
{
  if (_isRunning)
    {
      timeb now;
      _ftime(now);
      unsigned long seconds = _totalStartToStopSeconds + 
  static_cast<unsigned long>(now.time - _lastStartTime.time);
      
      return seconds * 1000 + _additionalTotalStartToStopMilliseconds
  + static_cast<long>(now.millitm)
  - static_cast<long>(_lastStartTime.millitm);       
    }
  else
    {
      return _totalStartToStopSeconds * 1000 
  + _additionalTotalStartToStopMilliseconds;
    }
}

void
Timer
::_ftime( timeb& theTime ) const
{
#ifndef DARWIN_SPECIFIC
  ftime( &theTime );
#else
  struct timeval timeVal;
  struct timezone timeZone;
  gettimeofday( &timeVal, &timeZone );
  theTime.time = timeVal.tv_sec;
  theTime.millitm = timeVal.tv_usec / 1000;
  theTime.timezone = timeZone.tz_minuteswest;
  theTime.dstflag = timeZone.tz_dsttime;
#endif
}
