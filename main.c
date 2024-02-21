#include <stdint.h>

//Peripheral & bus address
#define PERIPHERAL_BASE_ADDRESS 0x40000000U
#define AHB_BASE_ADDRESS 	(PERIPHERAL_BASE_ADDRESS + 0x00020000U)

//RCC BASE ADDRESS
#define RCC_BASE_ADDRESS	(AHB_BASE_ADDRESS + 0x00001000U)
#define RCC_IOPENR_ADDRESS	(RCC_BASE_ADDRESS + 0x0000002CU)

//IOPORT BASE ADDRESS
#define IOPORT_BASE_ADDRESS	(PERIPHERAL_BASE_ADDRESS + 0x10000000U)
#define GPIOC_BASE_ADDRESS (IOPORT_BASE_ADDRESS + 0x00000800U)
#define GPIOC_MODER_REG	(GPIOC_BASE_ADDRESS + 0x00000000U)
#define GPIOC_ODR_REG (GPIOC_BASE_ADDRESS + 0x00000014U)
#define GPIOC_IDR_REG (GPIOC_BASE_ADDRESS + 0x00000010U)
#define GPIOC_PUPD_REG (GPIOC_BASE_ADDRESS + 0x0000000CU)

void delay_ms(uint16_t n);

int main(void)
{
    /* Loop forever */
	uint32_t *ptr_rcc_iopenr =  RCC_IOPENR_ADDRESS; //pointer iopenr
	uint32_t *ptr_gpioc_moder = GPIOC_MODER_REG; // Moder gpioc
	uint32_t *ptr_gpioc_idr = GPIOC_IDR_REG; //Pointer variable gpioc
	uint32_t *ptr_gpioc_odr = GPIOC_ODR_REG; //Odr gpioc

	//Clock
	*ptr_rcc_iopenr |= 1<<2;//habilita el clock de GPIOC

	//Salidas
	//GPIOC PC9
	*ptr_gpioc_moder &= ~(1<<19);//Pin de salida

	//Entradas
	//GPIOC PC7
	*ptr_gpioc_moder &= ~(1<<14);//Pin de entrada entrada
	*ptr_gpioc_moder &= ~(1<<15);//Pin de entrada entrada

	//GPIOC PC8
	*ptr_gpioc_moder &= ~(1<<16);//Pin de entrada entrada
	*ptr_gpioc_moder &= ~(1<<17);//Pin de entradaentrada


	uint8_t i=0; //contador

	while(1){
		if(*ptr_gpioc_idr & (1<<8)   ){ //Enciende PC8
				*ptr_gpioc_odr &= ~(1<<9);
					delay_ms(200);
				*ptr_gpioc_odr	|= 1<<9;
					delay_ms(200);
			}
			else{
				*ptr_gpioc_odr &= ~(1<<9);
				delay_ms(500);
				*ptr_gpioc_odr	|= 1<<9;
				delay_ms(500);

			}

		if(*ptr_gpioc_idr & (1<<7)  )//Enciende PC8
					{

					i++;
					printf("Valor de contador: %d\n", i); //Cantidad del contador
				}

	}

}
void delay_ms(uint16_t n){
		uint16_t i;
		for(; n>0; n--)
			for (i=0; i<140; i++);

	}
