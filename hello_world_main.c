#include <stdio.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/mcpwm_types.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "soc/clk_tree_defs.h"

// a onda tem 100kHz, porém é dividida em 100 partes, logo a resolução 10000000

// definicoes
#define TIMER_USADO 0
#define RESOLUCAO 10000000
#define NUM_TICKS 100
#define DUTY_TICK 50
#define DELAY_TICKS 1

#define TRUE 1
#define FALSE 0

#define PA 12
#define PB 14

void app_main(void)
{
	// relogio
	mcpwm_timer_config_t timer_0_cfg = {
		.group_id = TIMER_USADO,
		.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
		.resolution_hz = RESOLUCAO,
		.count_mode = MCPWM_TIMER_COUNT_MODE_UP,
		.period_ticks = NUM_TICKS	
	};
	mcpwm_timer_handle_t timer_0 = NULL;
	mcpwm_new_timer(&timer_0_cfg, &timer_0);
	printf("relogio configurado\n");
	// operador
	mcpwm_operator_config_t oper_0_cfg = {
		.group_id = TIMER_USADO,
		.flags.update_gen_action_on_tez = TRUE
	};
	mcpwm_oper_handle_t oper_0 = NULL;
	mcpwm_new_operator(&oper_0_cfg, &oper_0);
	mcpwm_operator_connect_timer(oper_0, timer_0);
	printf("operador configurado\n");
	
	// comparador
	mcpwm_comparator_config_t cmpr_0_cfg = {
		.flags.update_cmp_on_tez = TRUE
	};
	mcpwm_cmpr_handle_t cmpr_0 = NULL;
	mcpwm_new_comparator(oper_0, &cmpr_0_cfg, &cmpr_0);
	mcpwm_comparator_set_compare_value(cmpr_0, DUTY_TICK);
	
	// gerador 0
	mcpwm_generator_config_t gen_0_cfg = {
		.gen_gpio_num = PA
	};
	mcpwm_gen_timer_event_action_t gen_0_time_action = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.event = MCPWM_TIMER_EVENT_FULL,
		.action = MCPWM_GEN_ACTION_HIGH
	};
	mcpwm_gen_compare_event_action_t gen_0_cmpr_action = {
		.direction = MCPWM_TIMER_DIRECTION_UP,
		.comparator = cmpr_0,
		.action = MCPWM_GEN_ACTION_LOW
	};
	mcpwm_gen_handle_t gen_0 = NULL;
	mcpwm_new_generator(oper_0, &gen_0_cfg, &gen_0);
	mcpwm_generator_set_action_on_timer_event(gen_0, gen_0_time_action); 
	mcpwm_generator_set_action_on_compare_event(gen_0, gen_0_cmpr_action);
	
	// gerador 1
	mcpwm_generator_config_t gen_1_cfg = {
		.gen_gpio_num = PB,
	};
	mcpwm_gen_handle_t gen_1 = NULL;
	mcpwm_new_generator(oper_0, &gen_1_cfg, &gen_1);
	mcpwm_generator_set_action_on_timer_event(gen_1, gen_0_time_action); 
	mcpwm_generator_set_action_on_compare_event(gen_1, gen_0_cmpr_action);
	
	// tempo morto
	mcpwm_dead_time_config_t dead_time_config = {
		.posedge_delay_ticks = DELAY_TICKS,
		.negedge_delay_ticks = 0
	};
	mcpwm_generator_set_dead_time(gen_0, gen_0, &dead_time_config);
	dead_time_config.posedge_delay_ticks = 0;
	dead_time_config.negedge_delay_ticks = DELAY_TICKS;
	dead_time_config.flags.invert_output = TRUE;
	mcpwm_generator_set_dead_time(gen_0, gen_1, &dead_time_config);
	
	mcpwm_timer_enable(timer_0);
	mcpwm_timer_start_stop(timer_0, MCPWM_TIMER_START_NO_STOP);
	printf("relogio rodando, pwm gerado.\n");
	
	return;	
}
