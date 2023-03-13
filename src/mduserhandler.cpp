#include "ThostFtdcMdApi.h"
#include "mduserhandler.h"
#include "params.h"
#include <cstring>
#include <iostream>
#include "unistd.h"
#include <fstream>
#include <sys/io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

// 初始化Api
void CMduserHandler::connect()
{
    std::cout << "----Handler connecting----" << std::endl;
    //在当前目录创建流文件夹
    if (access("../flow_md/", 0) == -1)
    {
        mkdir("../flow_md/", S_IRWXU);
    }
    //创建Api并初始化
    m_mdApi = CThostFtdcMdApi::CreateFtdcMdApi("../flow_md/", false, false); // 默认当前目录为什么是src？
    m_mdApi->RegisterSpi(this);                                             // 将Spi注册给Api，Api能将收到的数据通过Spi的接口回调给用户
    m_mdApi->RegisterFront(const_cast<char *>(pszFrontAddress));
    m_mdApi->Init(); // 正式初始化Api, 此时Api向之前注册的地址发起与CTP前置的连接，
    // 与前置连接成功后回调OnFrontConnected, 有时候比较慢，需要等会
    extern int connectFlag;
    std::cout << "waiting OnFrontConnected" << std::endl;
    while (connectFlag != 0)
    {
        sleep(3);
    }
    std::cout << "----Handler connected----" << std::endl;
}
// 登陆
void CMduserHandler::login()
{
    std::cout << "----Handler logging----" << std::endl;
    // 初始化登陆结构体，再赋值相应字段。对于行情的登录来说只需要调用登录接口即可，目前CTP暂时不对行情做密码校检
    CThostFtdcReqUserLoginField pReqUserLoginField = {0};
    int loginRet;
    strcpy(pReqUserLoginField.BrokerID, myBrokerID);
    strcpy(pReqUserLoginField.UserID, myUserID);
    strcpy(pReqUserLoginField.Password, myPassword);          // 运行完此处才调用OnFrontConnected，给了密码
    loginRet = m_mdApi->ReqUserLogin(&pReqUserLoginField, 1); // 运行完此处才调用OnRspUserLogin
    if (loginRet == 0)
    {
         std::cout << "waiting OnRspUserLogin" << std::endl;
        while (loginFlag != 0)
        {
            sleep(3);
        }
        std::cout << "----Handler logged----" << std::endl;
    }
    else
    {
        std::cout << "Handler login error: " << loginRet << std::endl;
        std::cout << "若不在交易时间段内，请更换为第二套行情前置地址" << std::endl;
        return;
    }
   
}
// 订阅行情
void CMduserHandler::subscribe()
{
    extern TThostFtdcInstrumentIDType targetInstrumentID[];
    extern int targetInstrumentIDNum;
    std::cout << "----Handler subscribing----" << std::endl;
    //此处怎么优化？
    char **ppInstrument = new char *[targetInstrumentIDNum];
    memset(ppInstrument, 0, sizeof(ppInstrument));
    for (int i = 0; i < targetInstrumentIDNum; i++)
    {
        ppInstrument[i] = new char[81];
        strcpy(ppInstrument[i], targetInstrumentID[i]);
    }
    int subscribeRet;
    // 订阅合约的数量需要改！
    subscribeRet = m_mdApi->SubscribeMarketData(ppInstrument, targetInstrumentIDNum);
    if (subscribeRet == 0)
    {
        std::cout << "----Handler subscribed----" << std::endl;
    }
    else
    {
        std::cout << "Handler subscribe error: " << subscribeRet << std::endl;
    }
    //释放内存
    for (int i = 0; i < targetInstrumentIDNum; i++)
    {
        if (ppInstrument[i] != NULL)
        {
            delete[] ppInstrument[i];
        }
    }
    if (ppInstrument != NULL)
    {
        delete[] ppInstrument;
    }
    //订阅请求发送成功后回调OnRspSubMarketData
}
// Api的Join
void CMduserHandler::join()
{
    std::cout << "Api calling Join()..." << std::endl;
    this->m_mdApi->Join();
    std::cout << "Api called Join()..." << std::endl;
}

// 重写纯虚函数
void CMduserHandler::OnFrontConnected()
{
    extern int connectFlag;
    connectFlag = 0;
    std::cout << "OnFrontConnected..." << std::endl;
    //在上级目录创建存放行情数据的文件夹
    if (access("../../MarketData/", 0) == -1)
    {
        mkdir("../../MarketData/", S_IRWXU);
    }
}

// 当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
void CMduserHandler::OnFrontDisconnected(int nReason)
{
    std::cout << "OnFrontDisconnected with reason: " << nReason << std::endl;
};

/// 心跳超时警告。当长时间未收到报文时，该方法被调用。
///@param nTimeLapse 距离上次接收报文的时间
void CMduserHandler::OnHeartBeatWarning(int nTimeLapse)
{
    std::cout << "OnHeartBeatWarning, 距离上次接收报文的时间: " << nTimeLapse << std::endl;
};

/// 登录请求响应
void CMduserHandler::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    std::cout << "OnRspUserLogin..." << std::endl;
    if (pRspInfo->ErrorID != 0)
    {
        std::cout << "ErrorMsg" << pRspInfo->ErrorMsg << std::endl;
    }
    else
    {
        extern int loginFlag;
        loginFlag = 0;
        //成功登录
        std::cout << "UserLogin Successful!" << std::endl;
        std::cout
            << "CurrentTradingDay: " << pRspUserLogin->TradingDay << std::endl;
        //首次登录成功时，创建文件夹存放该日数据
        extern char tradingDayDataPath[50];
        char tmpDatapath[50] = {0};
        strcpy(tmpDatapath, tradingDayDataPath);
        strcat(tmpDatapath, pRspUserLogin->TradingDay);
        if (access(tmpDatapath, 0) == -1)
        {
            int mkdirRet = 0;
            std::cout << "creating folder for tradingday " << pRspUserLogin->TradingDay << "...";
            mkdirRet = mkdir(tmpDatapath, S_IRWXU);
            if (mkdirRet == 0)
            {
                std::cout << "Done" <<std::endl;
            }
            else
            {
                std::cout << "creating folder Error" <<std::endl;
            }
            for (int i = 0; i < targetInstrumentIDNum; i++)
            {
                extern char tradingDayDataPath[50];
                char tmpDatapath[50] = {0};
                strcpy(tmpDatapath, tradingDayDataPath);
                strcat(tmpDatapath, pRspUserLogin->TradingDay);
                strcat(tmpDatapath, "/");
                strcat(tmpDatapath, targetInstrumentID[i]);
                strcat(tmpDatapath, ".csv");
                std::ofstream ofs(tmpDatapath, std::ios::out);
                if (!ofs.is_open())
                {
                    std::cout << "open file failed in OnRspUserLogin" << std::endl;
                    return;
                }
                ofs <<
                "TradingDay" << "," << "reserve1" << "," << "ExchangeID" << ","<<"reserve2" << "," << 
                "LastPrice" << "," << "PreSettlementPrice" << "," << "PreClosePrice" << "," << 
                "PreOpenInterest" << "," << "OpenPrice" << "," << "HighestPrice" << "," << 
                "LowestPrice" << "," << "Volume" << "," << "Turnover" << "," << "OpenInterest" << "," << 
                "ClosePrice" << "," << "SettlementPrice" << "," << "UpperLimitPrice" << "," << "LowerLimitPrice" << "," <<
                "PreDelta" << "," << "CurrDelta" << "," << "UpdateTime" << "," << "UpdateMillisec" << "," <<
                "BidPrice1" << "," << "BidVolume1" << "," << "AskPrice1" << "," << "AskVolume1" << "," << 
                "BidPrice2" << "," << "BidVolume2" << "," << "AskPrice2" << "," << "AskVolume2" << "," << 
                "BidPrice3" << "," << "BidVolume3" << "," << "AskPrice3" << "," << "AskVolume3" << "," << 
                "BidPrice4" << "," << "BidVolume4" << "," << "AskPrice4" << "," << "AskVolume4" << "," << 
                "BidPrice5" << "," << "BidVolume5" << "," << "AskPrice5" << "," << "AskVolume5" << "," << 
                "AveragePrice" << "," << "ActionDay" << "," << "nstrumentID" << "," << "ExchangeInstID" << "," << 
                "BandingUpperPrice" << "," << "BandingLowerPrice" 
                << std::endl;

            }
        }
        
        

    }
};

/// 登出请求响应
void CMduserHandler::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

};

/// 请求查询组播合约响应
void CMduserHandler::OnRspQryMulticastInstrument(CThostFtdcMulticastInstrumentField *pMulticastInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

};

/// 错误应答
void CMduserHandler::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

};

/// 订阅行情应答
void CMduserHandler::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    std::cout << "OnRspSubMarketData..." << std::endl;
    // 为什么只能输出两个字段？
    if (pRspInfo->ErrorID != 0)
    {
        std::cout << "RspInfo: " << pRspInfo->ErrorMsg << std::endl;
    }
    else
    {
        std::cout << "UserSubscirbe Successful!" << std::endl;
        std::cout << "InstrumentID: " << pSpecificInstrument->InstrumentID << std::endl;
    }
    
}

/// 取消订阅行情应答
void CMduserHandler::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

};

/// 订阅询价应答
void CMduserHandler::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

};

/// 取消订阅询价应答

void CMduserHandler::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){

};

/// 深度行情通知
void CMduserHandler::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    std::cout << "OnRtnDepthMarketData..." << std::endl;
    std::cout << "Returning " << pDepthMarketData->InstrumentID << std::endl;
    for (int i = 0; i < targetInstrumentIDNum; i++)
    {
        if (strcmp(pDepthMarketData->InstrumentID, targetInstrumentID[i]) == 0)
        {
            extern char tradingDayDataPath[50];
            char tmpDatapath[50] = {0};
            strcpy(tmpDatapath, tradingDayDataPath);
            strcat(tmpDatapath, pDepthMarketData->TradingDay);
            strcat(tmpDatapath, "/");
            strcat(tmpDatapath, targetInstrumentID[i]);
            strcat(tmpDatapath, ".csv");
            std::ofstream ofs(tmpDatapath, std::ios::app);
            if (!ofs.is_open())
            {
                std::cout << "open file failed" << std::endl;
                return;
            }
            else
            {
                
                ofs << pDepthMarketData->TradingDay << ","
                << pDepthMarketData->LastPrice << "," << pDepthMarketData->PreSettlementPrice << ","
                << pDepthMarketData->PreClosePrice << "," << pDepthMarketData->PreOpenInterest << ","
                << pDepthMarketData->OpenPrice << "," << pDepthMarketData->HighestPrice << ","
                << pDepthMarketData->LowestPrice << "," << pDepthMarketData->Volume << ","
                << pDepthMarketData->Turnover << "," << pDepthMarketData->OpenInterest << ","
                << pDepthMarketData->ClosePrice << "," << pDepthMarketData->SettlementPrice << ","
                << pDepthMarketData->UpperLimitPrice << "," << pDepthMarketData->LowerLimitPrice << ","
                << pDepthMarketData->PreDelta << "," << pDepthMarketData->CurrDelta << ","
                << pDepthMarketData->UpdateTime << "," << pDepthMarketData->UpdateMillisec << ","
                << pDepthMarketData->BidPrice1 << "," << pDepthMarketData->BidVolume1 << ","
                << pDepthMarketData->AskPrice1 << "," << pDepthMarketData->AskVolume1 << ","
                //以下盘口不可见
                << pDepthMarketData->BidPrice2 << "," << pDepthMarketData->BidVolume2 << ","
                << pDepthMarketData->AskPrice2 << "," << pDepthMarketData->AskVolume2 << ","
                << pDepthMarketData->BidPrice3 << "," << pDepthMarketData->BidVolume3 << ","
                << pDepthMarketData->AskPrice3 << "," << pDepthMarketData->AskVolume3 << ","
                << pDepthMarketData->BidPrice4 << "," << pDepthMarketData->BidVolume4 << ","
                << pDepthMarketData->AskPrice4 << "," << pDepthMarketData->AskVolume4 << ","
                << pDepthMarketData->BidPrice5 << "," << pDepthMarketData->BidVolume5 << ","
                << pDepthMarketData->AskPrice5 << "," << pDepthMarketData->AskVolume5 << ","

                << pDepthMarketData->AveragePrice << "," << pDepthMarketData->ActionDay << ","
                << pDepthMarketData->InstrumentID << "," << pDepthMarketData->ExchangeInstID << ","
                << pDepthMarketData->BandingUpperPrice << "," << pDepthMarketData->BandingLowerPrice << std::endl;
            }
        }
    }
};
/// 询价通知
void CMduserHandler::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
;
}
    