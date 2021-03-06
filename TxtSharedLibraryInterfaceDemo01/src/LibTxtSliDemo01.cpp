/*
 * Shared Library Interface
 * Demo about the use of the Transfer Area and the Ir (Joystick) part
 * started 2018-09-16
 * version 1.1.1.1
 * version 1.1.1.2 2018-09-23 resolve some bugs, M1..M4, and Joysticks
 * version 1.1.1.3 2018-11-16 resolve some bugs, M1..M4, and Joysticks
 * version 1.1.1.4 2019-05-04 adjust some details in the algoritme
 *
 * For fischertechnik GmbH by ing. C. van Leeuwen Btw.
 */
#include <stdio.h>          // for printf()
#include <unistd.h>         // for sleep()
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;
#include "KeLibTxtDl.h"          // TXT Lib
#include "FtShmem.h"
#include "XYTransformer.h"
/*
 * Some parts of this demo works only in the local mode.
 * The problem in the online mode is that the "Motor IOLib Thread" is not
 * running.
 * How this thread could be activate is part of research [2018-09-15]
 *
 */
/* static variables
 * note 01: The library is loaded on the first time of use and note removed from
 *          the memory after the ending of the program.
 *          So the IsInit can be true at the restart of an RoboPro program.
 * note 02: The stdout and stderr kan be see by using a SSH console and the
 *          screen command (screen -r for example
 *          https://www.computerhope.com/unix/screen.htm
 *          https://www.youtube.com/watch?v=Ernqw56u7PE
 * note 03: ssh connection putty tool
 * 	    https://www.chiark.greenend.org.uk/~sgtatham/putty/
 * note 04: ftp file transfer tools: for example:
 *          https://filezilla-project.org/
 *          or https://winscp.net
 */
static bool IsInit = false;
static INT16 motorRightPower=0,motorLeftPower=0; //power [-512..512]
static INT16 motorRightDirection=0, motorLeftDirection=0; //direction [CCW=-1, halt=0, CW=1]
static INT16 transformer=1;
static INT16 motorIdLeft=1, motorIdRight=2;// range [1,2,3,4]
                                       //constrain motorIdLeft!=motorIdRight=2

// Common debugging stuff for RoboProLib
unsigned int DebugFlags;
FILE *DebugFile;

extern "C" {

  // Return value:
  //  0: success, continue with waiting for pFinishVar becoming 1
  //  1: not finished
  //  2: busy (entity locked by other process)
  // -1: error
  // Other positive values can be used for other waiting codes
  // Other negative values can be used for other error codes

  int init(short* t)
  {
    if(!IsInit)
      {
	motorRightPower=-1;
	motorRightDirection=0; //direction [CCW=-1, halt=0, CW=1]
	motorLeftPower=1;
	motorLeftDirection=0; //direction [CCW=-1, halt=0, CW=1]
	IsInit = true;
	printf( "*****SliDemo01.init: Set has been done\n");
      }
    else
      {
	printf( "SliDemo01.init: Was already done ");
      }
    return 0;
  }

  /* First example
   * Recalculate based on X and Y the power for the left and right motor
   *
   *  Sets  the motorLeftPower and motorRightPower
    * These variables are accessible with: getMotorRightPowerShort
    *  and getMotorLeftPowerShort.
    *  input: transformation algorithme [0..3]
   */

  int setJoystickShort(INT16 v)
  {
    bool rr;
    FISH_X1_TRANSFER    *pTArea;

  if( !IsInit )
    {
      fprintf(stderr, "SliDemo01:setJoystickShort: Error not initialized!\n");
      return -1;
    }
  if( v<0 || v>3 )
    {
      fprintf(stderr, "SliDemo01:setJoystickShort: Error, transformation ID is out of range!\n");
      return -2;
    }
  pTArea = GetKeLibTransferAreaMainAddress();
  if (!pTArea)
	  {
	    fprintf(stderr, "SliDemo2.setJoystickShort: Error, TA is not available!\n");
	    return -1;
	  }
  INT16 varX = pTArea->sTxtInputs.sIrInput[v].i16JoyLeftX ;                  //
  INT16 varY = pTArea->sTxtInputs.sIrInput[v].i16JoyLeftY ;                  //
  int PowerLeft=0, PowerRight=0;
  rr= fischertechnik::joystick::XYTransformer::transformXYtoLeftRight(varX, varY, PowerLeft,  PowerRight,transformer);
  //  rr= fischertechnik::joystick::XYTransformer::transformXYtoLeftRightSimple(varX, varY, PowerLeft,  PowerRight);
  if(!rr)
    {        fprintf(stderr, "SliDemo01.setJoystickShort:Error in  XYTransformation \n");
    return -1;
    }
  motorLeftPower=PowerLeft;motorRightPower=PowerRight;
  printf( "SliDemo01.getJoystickShort:                     joystick Left [%d , %d] power [%d , %d]\n", varX, varY, PowerLeft, PowerRight);

  return 0;


  }
  /* First example, choice of motors will be done in the RoboPro program self.
   * The result of the calculation in Joystick for the left motor [-512..0..512]
   */
  int getMotorLeftPowerShort(INT16* v)
  {
    if( !IsInit )
      {
	fprintf(stderr, "SliDemo01.getMotorLeftPowerShort: Not initialized!\n");
	return -1;
      }
    *v = motorLeftPower;
    printf( "SliDemo01.getMotorLeftPowerShort: %d\n", *v);
    return 0;
  }
  /* First example, choice of motors will be done in the RoboPro program self.
   * The result of the calculation in Joystick for the right motor [-512..0..512]
   */
  int getMotorRightPowerShort(INT16* v)
  {
    if( !IsInit )
      {
	fprintf(stderr, "SliDemo01.getMotorRightPowerShort: Not initialized!\n");
	return -1;
      }
    *v = motorRightPower;
    printf( "SliDemo01.getMotorRightPowerShort: %d\n", *v);
    return 0;
  }
  /*
   *
   */
  int setTransformerShort(INT16 v)
  {
    if( !IsInit )
      {
	fprintf(stderr, "SliDemo01.setTransformerShort: Error not initialized!\n");
	return -1;
      }
    if( v<0 || v>5)
      {
	fprintf(stderr, "SliDemo01.setTransformerShort: Error wrong transformer!\n");
	return -1;
      }
    transformer=v;
    printf( "SliDemo01.setTransformerShort: transformer code = %d\n", v);
    return 0;
  }


  /*  For the second example
   *  Set which motor is in use as left motor [1..4]
   *  uses: transformer,
   */
  int setMotorIdLeftShort(INT16 v)
  {
    if( !IsInit )
      {
	fprintf(stderr, "SliDemo01.setMotorLeftShort: Error not initialized!\n");
	return -1;
      }
    if( v<1 || v>IZ_MOTOR  )
      {
	fprintf(stderr, "SliDemo01.setMotorLeftShort: Error wrong motor!\n");
	return -1;
      }
    motorIdLeft=v;
    printf( "SliDemo01.setMotorLeftShort: motor left = M%d\n", v);
    return 0;
  }
  /*  For the second example
     *  Set which motor is in use as right motor [1..4]
     */
  int setMotorIdRightShort(INT16 v)
  {
    if( !IsInit )
      {
	fprintf(stderr, "SliDemo01.setMotorRightShort: Error not initialized!\n");
	return -1;
      }
    if( v<1 || v>IZ_MOTOR  )
      {
	fprintf(stderr, "SliDemo01.setMotorRightShort: Error wrong motor!\n");
	return -1;
      }
    motorIdRight=v;
    printf( "SliDemo01.setMotorRightShort: motor right = M%d\n", v);
    return 0;
  }
  /* Second example
    * Recalculate based on X and Y the power for the left and right motor
    * and send this information directly to the motors.
    * Sets also the motorLeftPower and motorRightPower
    * These variables are accessible with: getMotorRightPowerShort
    *  and getMotorLeftPowerShort.
    * Note: runs only in the local mode.
    */

   int setJoystickMotorsShort(INT16 v)
   {
    bool rr;
    FISH_X1_TRANSFER    *pTArea;
   if( !IsInit )
     {
       fprintf(stderr, "SliDemo01.setJoystickMotorsShort: Not initialized!\n");
       return -1;
     }
   if( v<0 || v>NUM_OF_IR_RECEIVER )  //2018-09-22 was 3 and error
     {
       fprintf(stderr, "SliDemo01.setJoystickMotorsShort: Error, Joystick ID is out of range!\n");
       return -2;
     }

   pTArea = GetKeLibTransferAreaMainAddress();
   if (!pTArea)
	  {
	    fprintf(stderr, "SliDemo2.setJoystickMotorsShort: Error, TA is not available!\n");
	    return -1;
	  }

   INT16 varX = pTArea->sTxtInputs.sIrInput[v].i16JoyLeftX ;                  //
   INT16 varY = pTArea->sTxtInputs.sIrInput[v].i16JoyLeftY ;                  //
   int PowerLeft=0, PowerRight=0;
   rr= fischertechnik::joystick::XYTransformer::transformXYtoLeftRight(varX, varY, PowerLeft,  PowerRight,transformer);
   //  rr= fischertechnik::joystick::XYTransformer::transformXYtoLeftRightSimple(varX, varY, PowerLeft,  PowerRight);
    if (!rr)
      {
	fprintf (
	    stderr,
	    "SliDemo01.setJoystickMotorsShort:Error in  XYTransformation \n");
	return -3;
      }

    motorLeftPower = PowerLeft;
    motorRightPower = PowerRight;
    if (motorLeftPower >= 0)
      {
	pTArea->ftX1out.duty[(motorIdLeft - 1) * 2] = motorLeftPower; // Switch Motor 1 ( O1 [0] ) on with PWM Value MSpeed (= max speed)
	pTArea->ftX1out.duty[(motorIdLeft - 1) * 2 + 1] = 0; // Switch Motor 1 ( O2 [1] ) with minus
      }
    else
      {
	pTArea->ftX1out.duty[(motorIdLeft - 1) * 2] = 0; // Switch Motor 1 ( O1 [0] ) on with PWM Value MSpeed (= max speed)
	pTArea->ftX1out.duty[(motorIdLeft - 1) * 2 + 1] = abs (motorLeftPower); // Switch Motor 1 ( O2 [1] ) with minus
      }
    if (motorRightPower >= 0)
      {
	pTArea->ftX1out.duty[(motorIdRight - 1) * 2] = motorRightPower; // Switch Motor 1 ( O1 [0] ) on with PWM Value MSpeed (= max speed)
	pTArea->ftX1out.duty[(motorIdRight - 1) * 2 + 1] = 0; // Switch Motor 1 ( O2 [1] ) with minus
      }
    else
      {
	pTArea->ftX1out.duty[(motorIdRight - 1) * 2] = 0; // Switch Motor 1 ( O1 [0] ) on with PWM Value MSpeed (= max speed)
	pTArea->ftX1out.duty[(motorIdRight - 1) * 2 + 1] = abs ( motorRightPower);            // Switch Motor 1 ( O2 [1] ) with minus
      }
    printf (
	"SliDemo01.getJoystickShort: joystick Left [%d , %d] power [%d , %d]\n",
	varX, varY, PowerLeft, PowerRight);
    return 0;
   }
} // extern "C"
