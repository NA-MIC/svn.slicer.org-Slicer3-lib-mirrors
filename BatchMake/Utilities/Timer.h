/*=========================================================================

  Program:   BatchMake
  Module:    $RCSfile: Timer.h,v $
  Language:  C++
  Date:      $Date: 2004/11/30 20:04:23 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2005 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/

#ifndef Timer_h
#define Timer_h

#include <iostream>
#include <sys/timeb.h>

//
// simple timing class that acts like a stop watch
//
// bcd 2003
//

class Timer
{
public:
  Timer();

  // start timer if not already running
  void start();

  // stop timer if running
  void stop();

  // reset all values to 0, stop timer if running
  void reset();

  // get number of seconds timer has run
  unsigned long getSeconds() const;

  // get number of milliseconds timer has run
  unsigned long getMilliseconds() const;

private:
  bool           _isRunning;
  timeb          _lastStartTime;

  unsigned long  _totalStartToStopSeconds;
  long           _additionalTotalStartToStopMilliseconds;

  void _ftime( timeb& theTime ) const;
};
#endif
