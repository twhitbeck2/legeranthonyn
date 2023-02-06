/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "It_los_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

static void TaskF01(void)
{
    LOS_AtomicInc(&g_testCount);
    while (1) {
    }
}

static void TaskF02(void)
{
    UINT32 ret;
    LOS_AtomicInc(&g_testCount);

    ret = LOS_TaskYield();
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);

    TestBusyTaskDelay(2 * LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT); // 2, set delay time

    LOS_AtomicInc(&g_testCount);
}

static UINT32 Testcase(void)
{
    UINT32 ret;
    TSK_INIT_PARAM_S task1 = { 0 };
    UINT32 testTaskIDSmp[LOSCFG_KERNEL_CORE_NUM + 1];
    UINT64 timesliceCount1;
    UINT64 timesliceCount2;
    int i;
    const CHAR *taskAll = "-a";
    g_testCount = 0;
    TEST_TASK_PARAM_INIT(task1, "it_smp_task_132", (TSK_ENTRY_FUNC)TaskF01, TASK_PRIO_TEST_TASK);

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM - 1; i++) {
        /* take control of every cores */
        task1.usCpuAffiMask = CPUID_TO_AFFI_MASK((ArchCurrCpuid() + i + 1) % (LOSCFG_KERNEL_CORE_NUM));
        ret = LOS_TaskCreate(&testTaskIDSmp[i], &task1);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }

    TestBusyTaskDelay(10); // 10, set delay time

    /* check all task is running */
    ICUNIT_GOTO_EQUAL(g_testCount, LOSCFG_KERNEL_CORE_NUM - 1, g_testCount, EXIT);

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM - 1; i++) {
        do {
            OsShellCmdDumpTask(1, &taskAll);
            ret = OS_TCB_FROM_TID(testTaskIDSmp[i])->taskStatus;
        } while (ret & OS_TASK_STATUS_READY);
        ICUNIT_GOTO_EQUAL(ret, (OS_TASK_STATUS_RUNNING | OS_TASK_STATUS_DETACHED), ret, EXIT);
    }

    task1.usCpuAffiMask = 0;
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF02;
    ret = LOS_TaskCreate(&testTaskIDSmp[i], &task1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    while (g_testCount != LOSCFG_KERNEL_CORE_NUM) {
    }

    ICUNIT_GOTO_EQUAL(g_testCount, LOSCFG_KERNEL_CORE_NUM, g_testCount, EXIT);

    ret = LOS_TaskYield();
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ICUNIT_GOTO_EQUAL(g_testCount, LOSCFG_KERNEL_CORE_NUM, g_testCount, EXIT);

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        ret = LOS_TaskDelete(testTaskIDSmp[i]);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    }

    return LOS_OK;

EXIT:
    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        LOS_TaskDelete(testTaskIDSmp[i]);
    }

    return LOS_NOK;
}

void ItSmpLosTask132(void)
{
    TEST_ADD_CASE("ItSmpLosTask132", Testcase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
