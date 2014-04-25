/***********************************************************************
  版权信息 : 版权所有(C) 1988-2005, 华为技术有限公司.
  文件名   : pppd_thread.h
  作者     : lichangqing 45517
  版本     : V500R003
  创建日期 : 2005-8-8
  完成日期 : 2005-8-10
  功能描述 : 头文件
      
  主要函数列表: 无
      
  修改历史记录列表: 
    <作  者>    <修改时间>  <版本>  <修改描述>
    l45517      20050816    0.0.1    初始完成
  备注: 
************************************************************************/

#ifndef __PPPD_THREAD_H
#define __PPPD_THREAD_H

//A064D00348 qinzhiyuan begin
//int pppd_thread_initialize(int args,char** argv);
int pppd_thread_initialize(int args, const char** argv);
//A064D00348 qinzhiyuan end

int pppd_thread_destroy();

int pppd_thread_at_dail(int modem_fd);
int pppd_thread_ath(int modem_fd);

#endif //PPPD_THREAD_H

