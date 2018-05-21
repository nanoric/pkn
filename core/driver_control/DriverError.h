#pragma once

namespace pkn
{
    class kernel_open_error : public std::exception
    {};

    class kernel_ioctrl_error : public std::exception
    {};

    class kernel_read_memory_error : public kernel_ioctrl_error
    {};

    class kernel_write_memory_error : public kernel_ioctrl_error
    {};

    class kernel_get_physical_address_error : public kernel_ioctrl_error
    {};

    class kernel_write_physical_memory_error : public kernel_ioctrl_error
    {};

    class kernel_query_virtual_memory_error : public kernel_ioctrl_error
    {};

    class kernel_get_mapped_file_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_name_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_base_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_times_error : public kernel_ioctrl_error
    {};

    class kernel_get_process_exit_status_error : public kernel_ioctrl_error
    {};

    class kernel_get_mouse_pos_error : public kernel_ioctrl_error
    {};

    class kernel_synthesize_mouse_error : public kernel_ioctrl_error
    {};

    class kernel_wait_process_error : public kernel_ioctrl_error
    {};

    class kernel_not_implemented_error : public kernel_ioctrl_error
    {};
}
