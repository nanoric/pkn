#pragma once

NTSTATUS process_protect_install();
void process_protect_uninstall();
NTSTATUS protect_process(UINT64 pid);
NTSTATUS unprotecct_process();
