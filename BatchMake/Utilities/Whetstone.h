/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: Whetstone.h,v $
  Language:  C++
  Date:      $Date: 2007/05/04 11:11:49 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/* ADAPTED FROM THE FOLLOWING */

/**********************************************************/
/* Date: Mon, 10 Mar 1997 07:38:18 -0500                  */
/* From: Roy Longbottom <Roy_Longbottom@compuserve.com>   */
/* Subject: WHET02.txt                                    */
/* To: "Alfred A. Aburto Jr." <aburto@cts.com>            */
/**********************************************************/

/*
 *     C/C++ Whetstone Benchmark Single or Double Precision
 *
 *     Original concept        Brian Wichmann NPL      1960's
 *     Original author         Harold Curnow  CCTA     1972
 *     Self timing versions    Roy Longbottom CCTA     1978/87
 *     Optimisation control    Bangor University       1987/90
 *     C/C++ Version           Roy Longbottom          1996
 *     Compatibility & timers  Al Aburto               1996
 *
 ************************************************************
 *
 *              Official version approved by:
 *
 *         Harold Curnow  100421.1615@compuserve.com
 *
 *      Happy 25th birthday Whetstone, 21 November 1997
 *
 ************************************************************
 *
 *     The program normally runs for about 100 seconds
 *     (adjustable in main - variable duration). This time
 *     is necessary because of poor PC clock resolution.
 *     The original concept included such things as a given
 *     number of subroutine calls and divides which may be
 *     changed by optimisation. For comparison purposes the
 *     compiler and level of optimisation should be identified.
 *       
 ************************************************************
 *
 *     The original benchmark had a single variable I which
 *     controlled the running time. Constants with values up
 *     to 899 were multiplied by I to control the number
 *     passes for each loop. It was found that large values
 *     of I could overflow index registers so an extra outer
 *     loop with a second variable J was added.
 *
 *     Self timing versions were produced during the early
 *     days. The 1978 changes supplied timings of individual
 *     loops and these were used later to produce MFLOPS and
 *     MOPS ratings.
 *
 *     1987 changes converted the benchmark to Fortran 77
 *     standards and removed redundant IF statements and
 *     loops to leave the 8 active loops N1 to N8. Procedure
 *     P3 was changed to use global variables to avoid over-
 *     optimisation with the first two statements changed from
 *     X1=X and Y1=Y to X=Y and Y=Z. A self time calibrating
 *     version for PCs was also produced, the facility being
 *     incorporated in this version.
 *
 *     This version has changes to avoid worse than expected
 *     speed ratings, due to underflow, and facilities to show
 *     that consistent numeric output is produced with varying
 *     optimisation levels or versions in different languages.
 *
 *     Some of the procedures produce ever decreasing numbers.
 *     To avoid problems, variables T and T1 have been changed
 *     from 0.499975 and 0.50025 to 0.49999975 and 0.50000025.
 *
 *     Each section now has its own double loop. Inner loops
 *     are run 100 times the loop constants. Calibration
 *     determines the number of outer loop passes. The
 *     numeric results produced in the main output are for
 *     one pass on the outer loop. As underflow problems were
 *     still likely on a processor 100 times faster than a 100
 *     MHZ Pentium, three sections have T=1.0-T inserted in the
 *     outer loop to avoid the problem. The two loops avoid
 *     index register overflows.
 *
 *     The first section is run ten times longer than required
 *     for accuracy in calculating MFLOPS. This time is divided
 *     by ten for inclusion in the MWIPS calculations.
 *
 *     This version has facilities for typing in details of the
 *     particular run. This information is appended to file
 *     whets.res along with the results. The input section can
 *     be avoided using a command line parameter N (for example
 *     Whets.exe N).  
 *
 *     Roy Longbottom  101323.2241@compuserve.com
 *
 ************************************************************
 *
 *     Whetstone benchmark results are available in whets.tbl
 *     from ftp.nosc.mil/pub/aburto. The results include
 *     further details of the benchmarks.
 *
 ************************************************************
 *
 *     Source code is available in C/C++, Fortran, Basic and
 *     Visual Basic in the same format as this version. Pre-
 *     compiled versions for PCs are also available via C++.
 *     These comprise optimised and non-optimised versions
 *     for DOS, Windows and NT.
 *
 *     This version compiles and runs correctly either as a
 *     C or CPP program with a WATCOM and Borland compiler.
 *
 ************************************************************
 *
 * Example of initial calibration display (Pentium 100 MHz)
 *
 * Single Precision C/C++ Whetstone Benchmark
 *
 * Calibrate
 *      0.17 Seconds          1   Passes (x 100)
 *      0.77 Seconds          5   Passes (x 100)
 *      3.70 Seconds         25   Passes (x 100)
 *
 * Use 676  passes (x 100)
 *
 * 676 passes are used for an approximate duration of 100
 * seconds, providing an initial estimate of a speed rating
 * of 67.6 MWIPS.
 *
 * This is followed by the table of results as below. Input
 * statements are then supplied to type in the run details.
 *
 ************************************************************
 * 
 * Examples of results from file whets.res
 *
 * Whetstone Single  Precision Benchmark in C/C++
 *
 * Month run         4/1996
 * PC model          Escom
 * CPU               Pentium
 * Clock MHz         100
 * Cache             256K
 * H/W Options       Neptune chipset
 * OS/DOS            Windows 95
 * Compiler          Watcom C/C++ 10.5  Win386
 * Options           No optimisation
 * Run by            Roy Longbottom
 * From              UK
 * Mail              101323.2241@compuserve.com
 *
 * Loop content                 Result            MFLOPS     MOPS   Seconds
 *
 * N1 floating point    -1.12475025653839100      19.971              0.274
 * N2 floating point    -1.12274754047393800      11.822              3.240
 * N3 if then else       1.00000000000000000               11.659     2.530
 * N4 fixed point       12.00000000000000000               13.962     6.430
 * N5 sin,cos etc.       0.49904659390449520                2.097    11.310
 * N6 floating point     0.99999988079071040       3.360             45.750
 * N7 assignments        3.00000000000000000                2.415    21.810
 * N8 exp,sqrt etc.      0.75110864639282230                1.206     8.790
 *
 * MWIPS                                          28.462            100.134
 *
 * Whetstone Single  Precision Benchmark in C/C++
 *
 * Compiler          Watcom C/C++ 10.5  Win386
 * Options           -otexan -zp4 -om -fp5 -5r
 *
 * Loop content                 Result            MFLOPS     MOPS   Seconds
 *
 * N1 floating point    -1.12475025653839100      26.751              0.478
 * N2 floating point    -1.12274754047393800      17.148              5.220
 * N3 if then else       1.00000000000000000               19.922     3.460
 * N4 fixed point       12.00000000000000000               15.978    13.130
 * N5 sin,cos etc.       0.49904659390449520                2.663    20.810
 * N6 floating point     0.99999988079071040      10.077             35.650
 * N7 assignments        3.00000000000000000               22.877     5.380
 * N8 exp,sqrt etc.      0.75110864639282230                1.513    16.370
 *
 * MWIPS                                          66.270            100.498
 *
 *
 * Whetstone Double  Precision Benchmark in C/C++
 *
 * Compiler          Watcom C/C++ 10.5 Win32NT 
 * Options           -otexan -zp4 -om -fp5 -5r
 *
 * Loop content                 Result           MFLOPS      MOPS   Seconds
 *
 * N1 floating point    -1.12398255667391900     26.548               0.486
 * N2 floating point    -1.12187079889284400     16.542               5.460
 * N3 if then else       1.00000000000000000               19.647     3.540
 * N4 fixed point       12.00000000000000000               15.680    13.500
 * N5 sin,cos etc.       0.49902937281515140                3.019    18.520
 * N6 floating point     0.99999987890802820      9.977              36.330
 * N7 assignments        3.00000000000000000               22.620     5.490
 * N8 exp,sqrt etc.      0.75100163018457870                1.493    16.740
 *
 * MWIPS                                         67.156             100.066
 *
 *  Note different numeric results to single precision. Slight variations
 *  are normal with different compilers and sometimes optimisation levels. 
 *
 *
 *             Example Single Precision Optimised Results
 *
 *     MWIPS   MFLOPS  MFLOPS  MFLOPS  COS     EXP     FIXPT    IF    EQUAL
 * PC            1       2       3     MOPS    MOPS    MOPS    MOPS    MOPS
 *                                                                                                                                                
 * P3  5.68    0.928   0.884   0.673   0.461   0.275   2.36    2.16   0.638
 * P4  16.4    5.09    4.03    2.66    0.526   0.342   6.36    6.00    5.28
 * P5  66.3    26.8    17.1    10.1    2.66    1.51    16.0    19.9    22.9
 * P6  161     50.3    45.2    31.5    4.46    2.77    102     20.6    119
 *
 *            Example Single Precision Non-optimised Results
 *                                                                        
 * P3  3.07    0.860   0.815   0.328   0.355   0.160   1.70    1.32   0.264
 * P4  10.0    4.68    3.51    1.27    0.482   0.298   5.73    5.20    1.18
 * P5  28.5    20.0    11.8    3.36    2.10    1.21    14.0    11.7    2.42
 * P6  81.7    47.5    37.8    10.9    3.91    2.43    51.2    42.8    7.85
 *
 *        Summary results as in whets.tbl at ftp.nosc.mil/pub/aburto 
 *
 *           MFLOPS   = Geometric Mean of three MFLOPS loops
 *           VAX MIPS = 5 * Geometric Mean of last three items above
 *
 *                                                                     VAX
 * PC System  CPU/Options               Cache   MHz   MWIPS   MFLOPS  MIPS
 *
 * P3 Clone   AM80386DX with 387        128K    40    5.68    0.820   7.40
 * P4 Escom   80486DX2 CIS chipset      128K    66    16.4    3.79    29.3
 * P5 Escom   Pentium Neptune chipset   256K   100    66.3    16.7    96.9
 * P6 Dell    PentiumPro 440FX PCIset   256K   200    161     41.5    315 
 *
 * P3 Clone   AM80386DX with 387        128K    40    3.07    0.613   4.20
 * P4 Escom   80486DX2 CIS chipset      128K    66    10.0    2.75    16.4
 * P5 Escom   Pentium Neptune chipset   256K   100    28.5    9.26    36.6
 * P6 Dell    PentiumPro 440FX PCIset   256K   200    81.7    26.9    129
 *
 */
 
#ifndef WHETSTONES_H
#define WHETSTONES_H

/** \class WhetstoneClock
* \brief The WhetstoneClock provides a timestamp from a real-time clock
*
* This class represents a real-time clock object
* and provides a timestamp in platform-independent format.
*
* \author Hee-Su Kim, Compute Science Dept. Kyungpook National University,
*                     ISIS Center, Georgetown University.
*/

class WhetstoneClock
{
public:

  /** Define the type for the timestap */
  typedef double        TimeStampType;

  /** Define the type for the frequency of the clock */
  typedef double        FrequencyType;

  /** constructor */
  WhetstoneClock();

  /** destructor */
  virtual ~WhetstoneClock();

  /** Returns a timestamp in seconds   e.g. 52.341243 seconds */
  TimeStampType GetTimeStamp() const;

  /** Returns the frequency of a clock */
  FrequencyType GetFrequency() const;

private:
  FrequencyType    m_Frequency;
  TimeStampType    m_Difference;
  TimeStampType    m_Origin;
};

class Whetstone
  {
  public:
    typedef    double     RealType;

    Whetstone();
    ~Whetstone();

    void SetDesiredDurations( double calibrationSeconds,
                              double benchmarkSeconds );
    void SetDesiredCalibrationDuration( double calibrationSeconds );
    void SetDesiredBenchmarkDuration( double benchmarkSeconds );

    double GetMIPS( void );
    double GetMFLOPS( void );
    double GetMWIPS( void );

    double GetMCosOPS( void );
    double GetMExpOPS( void );
    double GetFixptOPS( void );
    double GetIfOPS( void );
    double GetEqualOPS( void );

    void ComputeBenchmark( void );

  protected:
    double m_DesiredCalibrationSeconds;
    double m_DesiredBenchmarkSeconds;

    void RunWhetstone(long xtra, double desiredTime, bool calibrate);  

    void Pa(RealType e[4], RealType t, RealType t2);
    void Po(RealType e1[4], long j, long k, long l);
    void P3(RealType *x, RealType *y, RealType *z,
            RealType t, RealType t1, RealType t2);
  
    double m_LoopTime[9];
    double m_LoopMOPS[9];
    double m_LoopMFLOPS[9];
    double m_MWIPS;
    double m_Check;
    double m_Results[9];

    double             m_TimeUsed;
    WhetstoneClock    m_Clock;

    void UpdateTime(double ops, int type, double checknum,
                    double time, bool calibrate, int section);
  };

#endif
