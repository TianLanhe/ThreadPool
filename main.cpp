#include "stockTask\ConfigureHelper.h"
#include "stockTask\StockTask.h"
#include "include\ThreadPool.h"
#include "include\ThreadPoolFactory.h"
#include <io.h>
#include <direct.h>
#include <iostream>

using namespace std;

int main() {
	ConfigureHelper conf("configure.txt");
	CHECK_ERROR_BY_MSG(conf.isValid(), "配置文件读取成功", "配置文件读取错误！");

	int N1 = conf.GetN1();
	int N1_init = conf.GetN1_Init();
	int N1_step = conf.GetN1_Step();
	CHECK_ERROR_BY_MSG2(N1_step > 0, "N1_step 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(N1_init > 0, "N1_init 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(N1 > 0, "N1 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(N1 >= N1_init, "N1 必须大于等于 N1_init！");

	int N2 = conf.GetN2();
	int N2_init = conf.GetN2_Init();
	int N2_step = conf.GetN2_Step();
	CHECK_ERROR_BY_MSG2(N2_step > 0, "N2_step 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(N2_init > 0, "N2_init 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(N2 > 0, "N2 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(N2 >= N2_init, "N2 必须大于等于 N2_init！");

	int money = conf.GetMoney();
	CHECK_ERROR_BY_MSG2(money > 0, "money 非法，必须大于0！");

	int holddays_max = conf.GetHoldDays_Max();
	int holddays_init = conf.GetHoldDays_Init();
	int holddays_step = conf.GetHoldDays_Step();
	CHECK_ERROR_BY_MSG2(holddays_step > 0, "holddays_step 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(holddays_init > 1, "holddays_init 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(holddays_max > 0, "holddays_max 非法，必须大于0！");
	CHECK_ERROR_BY_MSG2(holddays_max >= holddays_init, "holddays_max 必须大于等于 holddays_init！");

	int start_date = conf.GetStartDate();
	int end_date = conf.GetEndDate();
	CHECK_ERROR_BY_MSG2(start_date > 19000101, "start_date 非法，必须大于19000101！");
	CHECK_ERROR_BY_MSG2(end_date > 19000101, "end_date 非法，必须大于19000101！");
	CHECK_ERROR_BY_MSG2(end_date > start_date, "end_date 必须大于start_date！");

	int thread_num = conf.GetThreadNum();
	CHECK_ERROR_BY_MSG2(thread_num > 0 && thread_num < 1000, "thread_num 非法，必须大于0且小于1000！");

	double x = conf.GetX();
	CHECK_ERROR_BY_MSG2(x > 0, "x 非法，必须是大于0的实数");

	string dir_path = conf.GetReadFilePath();
	string out_path = conf.GetWriteFilePath();

	_finddata_t fileInfo;
	long fHandle = _findfirst((dir_path + "\\*.csv").c_str(), &fileInfo);
	CHECK_ERROR_BY_MSG(fHandle != -1, "成功打开 " + dir_path + " 文件夹", dir_path + " 目录下没有\".csv\"文件！");

	if (_access(out_path.c_str(), 0) != 0)
		CHECK_ERROR_BY_MSG(_mkdir(out_path.c_str()) == 0, "成功创建输出目录 " + out_path, "不存在目录 " + out_path + " 且创建失败！");

	BEFORE_CHECK_RESULT();
	ThreadPool *pool = ThreadPoolFactory::GetInstance()->GetThreadPool();
	{
		char n[4];
		_itoa(thread_num, n, 10);
		CHECK_RESULT_BY_MSG(pool->SetThreadsNum(thread_num), string("设置线程数目为 ") + n, "线程池设置线程数量错误！");
	}

	do {
		CHECK_RESULT_BY_MSG(pool->AddTask(new StockTask(dir_path + "/" + fileInfo.name, out_path,
			N1, N1_init, N1_step,
			N2, N2_init, N2_step,
			holddays_init, holddays_max, holddays_step,
			money,
			start_date, end_date,
			x)), string("添加文件 ") + dir_path + "/" + fileInfo.name, "线程池添加任务失败！");
	} while (_findnext(fHandle, &fileInfo) == 0);
	_findclose(fHandle);

	GlobalOutputHelper::InitCriticalSection();

	CHECK_RESULT_BY_MSG(pool->StartTasks(), "开始执行...", "执行任务失败！");
	CHECK_RESULT_BY_MSG(pool->WaitForThreads(), "所有文件计算结束", "任务失败！");

	CHECK_RESULT_BY_MSG(GlobalOutputHelper::GetInstance()->OutputToFile(out_path), "汇总记录输出完毕，任务执行成功", "任务失败！");
	GlobalOutputHelper::DestroyCriticalSection();
	return 0;
}

