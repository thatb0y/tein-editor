GLOBAL constexpr const char* DEBUG_LOG_NAME = "debug.log";

GLOBAL int   current_debug_section = 0;
GLOBAL FILE* debug_log;

struct Debug_Timer
{
    std::string  name;
    u64 start_counter;
};

GLOBAL std::stack<Debug_Timer> debug_timers;
GLOBAL std::vector<std::string> debug_timer_results;

STDDEF void internal__log_debug (const char* format, ...)
{
    // We only open the debug log once the first debug occurs.
    if (!debug_log)
    {
        std::string debug_log_name(get_appdata_path() + LOGS_PATH + DEBUG_LOG_NAME);
        create_path(strip_file_name(debug_log_name));
        debug_log = fopen(debug_log_name.c_str(), "w");
    }

    va_list args;

    #if defined(BUILD_DEBUG)
    va_start(args, format);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
    fflush(stdout);
    #endif

    if (debug_log)
    {
        va_start(args, format);
        for (int i=0; i<current_debug_section; ++i)
        {
            fprintf(debug_log, "  ");
        }
        vfprintf(debug_log, format, args);
        fprintf(debug_log, "\n");
        va_end(args);
        fflush(debug_log);
    }
}

FILDEF void quit_debug_system ()
{
    // This condition is important because, for some reason, calling
    // fclose(NULL) results in a long hang-time during program exit.
    if (debug_log)
    {
        fclose(debug_log);
        debug_log = NULL;
    }
}

#if defined(BUILD_DEBUG)
FILDEF void begin_debug_timer (const char* name)
{
    Debug_Timer timer = {};
    timer.start_counter = SDL_GetPerformanceCounter();
    timer.name = name;
    debug_timers.push(timer);
}
FILDEF void end_debug_timer ()
{
    Debug_Timer timer = debug_timers.top();
    debug_timers.pop();

    u64   start_counter = timer.start_counter;
    u64   end_counter   = SDL_GetPerformanceCounter();
    float frequency     = CAST(float, SDL_GetPerformanceFrequency());
    float elapsed       = CAST(float, end_counter - start_counter);
    float seconds       = (elapsed / frequency);

    LOG_DEBUG("%s took: %fs", timer.name.c_str(), seconds);
}
#else
FILDEF void begin_debug_timer (const char* name)
{
    // Nothing...
}
FILDEF void end_debug_timer ()
{
    // Nothing...
}
#endif
