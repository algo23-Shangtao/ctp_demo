#include "params.h"
TThostFtdcBrokerIDType myBrokerID = "9999";
int connectFlag = -1;
int loginFlag = -1;
char tradingDayDataPath[50] = "../../MarketData/";
//输入相关信息
TThostFtdcUserIDType myUserID = "210468";
TThostFtdcPasswordType myPassword = "Tt1234567890!";
//更改前置地址请取消注释
const char* pszFrontAddress = "tcp://180.168.146.187:10211";//第一套（支持上期所期权、能源中心期权、中金所期权） 交易阶段(服务时间)：与实际生产环境保持一致。
//const char* pszFrontAddress = "tcp://180.168.146.187:10131";//第二套（7*24）：交易阶段(服务时间)：交易日，16:00～次日09:00；非交易日，16:00～次日12:00。
int targetInstrumentIDNum = 2;//订阅合约数量
TThostFtdcInstrumentIDType targetInstrumentID[] = {"rb2303", "ag2303"};//订阅合约ID
