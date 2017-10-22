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
	CHECK_ERROR_BY_MSG(conf.isValid(), "�����ļ���ȡ�ɹ�", "�����ļ���ȡ����");

	int N1 = conf.GetN1();
	int N1_init = conf.GetN1_Init();
	int N1_step = conf.GetN1_Step();
	CHECK_ERROR_BY_MSG2(N1_step > 0, "N1_step �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(N1_init > 0, "N1_init �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(N1 > 0, "N1 �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(N1 >= N1_init, "N1 ������ڵ��� N1_init��");

	int N2 = conf.GetN2();
	int N2_init = conf.GetN2_Init();
	int N2_step = conf.GetN2_Step();
	CHECK_ERROR_BY_MSG2(N2_step > 0, "N2_step �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(N2_init > 0, "N2_init �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(N2 > 0, "N2 �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(N2 >= N2_init, "N2 ������ڵ��� N2_init��");

	int money = conf.GetMoney();
	CHECK_ERROR_BY_MSG2(money > 0, "money �Ƿ����������0��");

	int holddays_max = conf.GetHoldDays_Max();
	int holddays_init = conf.GetHoldDays_Init();
	int holddays_step = conf.GetHoldDays_Step();
	CHECK_ERROR_BY_MSG2(holddays_step > 0, "holddays_step �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(holddays_init > 1, "holddays_init �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(holddays_max > 0, "holddays_max �Ƿ����������0��");
	CHECK_ERROR_BY_MSG2(holddays_max >= holddays_init, "holddays_max ������ڵ��� holddays_init��");

	int start_date = conf.GetStartDate();
	int end_date = conf.GetEndDate();
	CHECK_ERROR_BY_MSG2(start_date > 19000101, "start_date �Ƿ����������19000101��");
	CHECK_ERROR_BY_MSG2(end_date > 19000101, "end_date �Ƿ����������19000101��");
	CHECK_ERROR_BY_MSG2(end_date > start_date, "end_date �������start_date��");

	int thread_num = conf.GetThreadNum();
	CHECK_ERROR_BY_MSG2(thread_num > 0 && thread_num < 1000, "thread_num �Ƿ����������0��С��1000��");

	double x = conf.GetX();
	CHECK_ERROR_BY_MSG2(x > 0, "x �Ƿ��������Ǵ���0��ʵ��");

	string dir_path = conf.GetReadFilePath();
	string out_path = conf.GetWriteFilePath();

	_finddata_t fileInfo;
	long fHandle = _findfirst((dir_path + "\\*.csv").c_str(), &fileInfo);
	CHECK_ERROR_BY_MSG(fHandle != -1, "�ɹ��� " + dir_path + " �ļ���", dir_path + " Ŀ¼��û��\".csv\"�ļ���");

	if (_access(out_path.c_str(), 0) != 0)
		CHECK_ERROR_BY_MSG(_mkdir(out_path.c_str()) == 0, "�ɹ��������Ŀ¼ " + out_path, "������Ŀ¼ " + out_path + " �Ҵ���ʧ�ܣ�");

	BEFORE_CHECK_RESULT();
	ThreadPool *pool = ThreadPoolFactory::GetInstance()->GetThreadPool();
	{
		char n[4];
		_itoa(thread_num, n, 10);
		CHECK_RESULT_BY_MSG(pool->SetThreadsNum(thread_num), string("�����߳���ĿΪ ") + n, "�̳߳������߳���������");
	}

	do {
		CHECK_RESULT_BY_MSG(pool->AddTask(new StockTask(dir_path + "/" + fileInfo.name, out_path,
			N1, N1_init, N1_step,
			N2, N2_init, N2_step,
			holddays_init, holddays_max, holddays_step,
			money,
			start_date, end_date,
			x)), string("����ļ� ") + dir_path + "/" + fileInfo.name, "�̳߳��������ʧ�ܣ�");
	} while (_findnext(fHandle, &fileInfo) == 0);
	_findclose(fHandle);

	GlobalOutputHelper::InitCriticalSection();

	CHECK_RESULT_BY_MSG(pool->StartTasks(), "��ʼִ��...", "ִ������ʧ�ܣ�");
	CHECK_RESULT_BY_MSG(pool->WaitForThreads(), "�����ļ��������", "����ʧ�ܣ�");

	CHECK_RESULT_BY_MSG(GlobalOutputHelper::GetInstance()->OutputToFile(out_path), "���ܼ�¼�����ϣ�����ִ�гɹ�", "����ʧ�ܣ�");
	GlobalOutputHelper::DestroyCriticalSection();
	return 0;
}

