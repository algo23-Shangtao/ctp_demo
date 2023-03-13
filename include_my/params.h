#pragma once
#include "ThostFtdcUserApiDataType.h"

extern TThostFtdcUserIDType myUserID;
extern TThostFtdcInvestorIDType myInvestID;
extern TThostFtdcBrokerIDType myBrokerID;
extern TThostFtdcPasswordType myPassword;
extern const char* pszFrontAddress;
//订阅合约ID、数量
extern int targetInstrumentIDNum;
extern TThostFtdcInstrumentIDType targetInstrumentID[];
//for sleep
extern int connectFlag;
extern int loginFlag;
extern char tradingDayDataPath[50];

