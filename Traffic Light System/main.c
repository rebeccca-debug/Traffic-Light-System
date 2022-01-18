#include "stm32f4xx.h"							// Device header
#include "uart.h"

#define TRAFFIC_LIGHTS_PORT				GPIOA
#define CAR_SENSORS_PORT					GPIOC

#define EAST_LED_RED							(1U<<9)
#define EAST_LED_YELLOW						(1U<<8)
#define EAST_LED_GREEN						(1U<<7)

#define NORTH_LED_RED							(1U<<6)
#define NORTH_LED_YELLOW					(1U<<5)
#define NORTH_LED_GREEN						(1U<<4)

#define EAST_LED_RED_MODE_BIT			(1U<<18)
#define EAST_LED_YELLOW_MODE_BIT	(1U<<16)
#define EAST_LED_GREEN_MODE_BIT		(1U<<14)

#define NORTH_LED_RED_MODE_BIT		(1U<<12)
#define NORTH_LED_YELLOW_MODE_BIT	(1U<<10)
#define NORTH_LED_GREEN_MODE_BIT  (1U<<8)

#define NORTH_SENSOR							(1U<<1)
#define EAST_SENSOR								(1U<<0)

//go_north, PA9-4 = 10001 makes it green on North and red on East
//wait_north, PA9-4 = 100010 makes it yellow on North and red on East
//go_east, PA9-4 = 001100 makes it red on North and green on East
//wait_east, PA9-4 = 010100 makes it red on North and yellow on East

struct State {
	void(*sysPtr)(void);
	uint32_t time;
	const struct State * next_state[4];
};

typedef const struct State stateType;

#define go_north		&STATE_MACHINE[0]
#define	wait_north	&STATE_MACHINE[1]
#define go_east			&STATE_MACHINE[2]
#define wait_east		&STATE_MACHINE[3]

void traffic_allow_north(void);
void traffic_wait_north(void);
void traffic_allow_east(void);
void traffic_wait_east(void);

stateType STATE_MACHINE[4] = {
		
	//Using 3sec and 0.5 sec for demo
		
	{&traffic_allow_north,3000,{go_north, wait_north, go_north, wait_north}},
	{&traffic_wait_north,500, {go_east,go_east,go_east,go_east}},
	{&traffic_allow_east,3000, {go_east,go_east,wait_east,wait_east}},
	{&traffic_wait_east,500, {go_north,go_north,go_north,go_north}},
};

void TrafficSystemInit();
void delayMs(int n);

stateType *statePtr;
uint8_t system_input;

int main(void){
	
	//TODO : Initialize hardware then set initial state
	TrafficSystemInit();
	statePtr = go_north;
	
	while(1){
		delayMs(statePtr->time);
		system_input = CAR_SENSORS_PORT->IDR & (NORTH_SENSOR | EAST_SENSOR);
		statePtr = statePtr->next_state[system_input];
	}
}

void TrafficSystemInit(){
	RCC->AHB1ENR != 0x01 | 0x04; //Enable clock access to PORTA and PORTC
	
	TRAFFIC_LIGHTS_PORT->MODER != NORTH_LED_GREEN_MODE_BIT | NORTH_LED_RED_MODE_BIT |
																NORTH_LED_YELLOW_MODE_BIT | EAST_LED_GREEN_MODE_BIT |
																EAST_LED_YELLOW_MODE_BIT | EAST_LED_RED_MODE_BIT;
	
	USART2_Init();
}

void delayMs(int n){
	/*Configure SysTick*/
	SysTick->LOAD = 16000; /*Reload with number of clocks per millisecond*/
	SysTick->VAL = 0;			/*Clear current value register*/
	SysTick->CTRL = 0x05;	/*Enable the timer*/
	
	for(int i = 0; i < n; i++){
		while ((SysTick->CTRL & 0x10000)==0){}	/*Wait until COUNTFLAG is set*/
	}
	
	SysTick->CTRL = 0;
}

void traffic_allow_north(void){
	printf("Cars are passing from the north....\r\n");
	TRAFFIC_LIGHTS_PORT->ODR = 0x210;
}
void traffic_wait_north(void){
	printf("Cars are waiting from the north....\r\n");
	TRAFFIC_LIGHTS_PORT->ODR = 0x220;
}
void traffic_allow_east(void){
	printf("Cars are passing from the east....\r\n");
	TRAFFIC_LIGHTS_PORT->ODR = 0x0C0;
}
void traffic_wait_east(void){
	printf("Cars are waiting from the east....\r\n");
	TRAFFIC_LIGHTS_PORT->ODR = 0x140;
}