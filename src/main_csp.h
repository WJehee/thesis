#define STACK_SIZE      ( configMINIMAL_STACK_SIZE * 20 )
#define TASK_PRIORITY   ( tskIDLE_PRIORITY + 1 )

enum Interface {
    UDP = 0,
    ZMQ = 1,
};

// Other
void setup_csp(void);
void setup_default_handlers(void);
void route_work(void* params);

