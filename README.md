--------基于CTP接口，实现接收期货level1 tick行情数据的小程序--------


1.如何使用：
在params.cpp中输入账号密码以及要订阅的合约ID：
myUserID,myPassword更改为自己的simnow账号，pszFrontAddress为连接交易所行情的前置地址，交易时间内使用第一套前置地址，收盘后使用第二套前置地址，注意在服务时间内使用，否则会无法登录
在targetInstrumentID中输入要订阅行情数据的合约代码
2.获取的数据存放：
运行后，在上级目录创建MarketData文件夹。每个交易日，其内部创建一个与当前交易日同名的文件夹，用来存储订阅合约的行情数据
数据表头的含义参考ThostFtdUserApiStruct.h文件中CThostFtdcDepthMarketDataField结构体，交易所只提供一档盘口数据

3.参考：
交易时间：参考simnow官网产品与服务页(simnow官网：https://www.simnow.com.cn/)
交易品种：六所所有期货品种以及上期所、能源中心、中金所所有期权品种。
期货合约代码参考：
上期所期货合约代码：https://www.shfe.com.cn/products/isin/
郑商所期货合约代码：http://www.czce.com.cn/cn/jysj/cksj/H770322index_1.htm
大商所期货合约代码：http://www.dce.com.cn/publicweb/businessguidelines/queryContractInfo.html
上期能源期货合约代码https://www.ine.com.cn/bourseService/summary/?name=currinstrumentprop
ps：中金所期货行情数据有准入门槛

