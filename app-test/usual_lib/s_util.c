#include "s_util.h"

static void report(const char *prefix, const char *err, va_list params)
{
	char msg[1024];
	vsnprintf(msg, sizeof(msg), err, params);
	fprintf(stderr, " %s%s\n", prefix, msg);
}

static NORETURN void die_builtin(const char *err, va_list params)
{
	report(" Fatal: ", err, params);
	exit(128);
}

static void error_builtin(const char *err, va_list params)
{
	report(" Error: ", err, params);
}

static void warn_builtin(const char *warn, va_list params)
{
	report(" Warning: ", warn, params);
}

static void info_builtin(const char *info, va_list params)
{
	report(" Info: ", info, params);
}

void print_die(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	die_builtin(err, params);
	va_end(params);
}

void die_perror(const char *s)
{
	perror(s);
	exit(1);
}


int print_err(const char *err, ...)
{
	va_list params;

	va_start(params, err);
	error_builtin(err, params);
	va_end(params);
	return -1;
}

void print_warning(const char *warn, ...)
{
	va_list params;

	va_start(params, warn);
	warn_builtin(warn, params);
	va_end(params);
}

void print_info(const char *info, ...)
{
	va_list params;

	va_start(params, info);
	info_builtin(info, params);
	va_end(params);
}




void host_env(struct host_env * henv)
{
    henv->nr_online_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	henv->nr_pages	= sysconf(_SC_PHYS_PAGES);
	henv->page_size	= sysconf(_SC_PAGE_SIZE);
    henv->mem_size  = (henv->nr_pages * henv->page_size) >> GB_SHIFT;
}



#ifdef MAIN_TEST
bool do_debug_print = true;

int main(int argc, char * argv[])
{
    struct host_env env;
    host_env(&env);
    pr_err("cpu num is %d, nr_pages %lu, page_size %lu, mem %ldG\n",
        env.nr_online_cpus,env.nr_pages, env.page_size, env.mem_size);


    return 0;
}
#endif

