#ifndef RTDS_CCM_H
#define RTDS_CCM_H

#define RTDS_PORT Settings::mRTDSportNo
#define RTDS_CCM Settings::mRTDSccmPortNo
#define RTDS_START_THREAD Settings::mRTDSthreadCount
#define NEED_TO_ABORT Settings::mNeedToAbort
#define SIGNAL_ABORT Settings::mNeedToAbort = true;

#include <string>
class Settings
{
/*******************************************************************************************
* @brief Find port number.
*
* @param[in]		Port number as string
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void mFindPortNumber(std::string);
	static void mFindccmPortNumber(std::string);
/*******************************************************************************************
* @brief Find Thread count.
*
* @param[in]		Thread cound as string
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void mFindThreadCount(std::string);
public:
	static unsigned short mRTDSportNo;			// RTDS port number
	static unsigned short mRTDSccmPortNo;		// RTDS CCM port number
	static short mRTDSthreadCount;				// Number of RTDS threads
	static bool mNeedToAbort;					// True if RTDS needs to be aborted
/*******************************************************************************************
* @brief Process Arguments string
*
* @param[in]		Argument string
*
* @details
* std::err will display the error in argument and exit if the arguments are incorrect.
********************************************************************************************/
	static void processArgument(std::string);
};
#endif
