#include <stdint.h>

// Definiciones de registros y direcciones
#define PERIPHERAL_BASE_ADDRESS     0x40000000U
#define AHB_BASE_ADDRESS            (PERIPHERAL_BASE_ADDRESS + 0x00020000U)
#define RCC_BASE_ADDRESS            (AHB_BASE_ADDRESS + 0x00001000U)
#define RCC_IOPENR_ADDRESS          (RCC_BASE_ADDRESS + 0x0000002CU)
#define IOPORT_ADDRESS              (PERIPHERAL_BASE_ADDRESS + 0x10000000U)
#define GPIOA_BASE_ADDRESS          (IOPORT_ADDRESS + 0x00000000U)
#define GPIOB_BASE_ADDRESS          (IOPORT_ADDRESS + 0x00000400U)
#define GPIOC_BASE_ADDRESS          (IOPORT_ADDRESS + 0x00000800U)

#define GPIOA ((GPIO_RegDef_t*)GPIOA_BASE_ADDRESS)
#define GPIOB ((GPIO_RegDef_t*)GPIOB_BASE_ADDRESS)
#define GPIOC ((GPIO_RegDef_t*)GPIOC_BASE_ADDRESS)
#define RCC ((RCC_RegDef_t*)RCC_BASE_ADDRESS)


int8_t	key_pressed = 	0x00;

typedef struct
{
    uint32_t MODER;
    uint32_t OTYPER;
    uint32_t OSPEEDR;
    uint32_t PUPDR;
    uint32_t IDR;
    uint32_t ODR;
    uint32_t BSRR;
    uint32_t LCKR;
    uint32_t AFR[2];
    uint32_t BRR;
} GPIO_RegDef_t;

typedef struct
{
    uint32_t CR;
    uint32_t ICSCR;
    uint32_t CRRCR;
    uint32_t CFGR;
    uint32_t CIER;
    uint32_t CIFR;
    uint32_t CICR;
    uint32_t IOPRSTR;
    uint32_t AHBRSTR;
    uint32_t APB2RSTR;
    uint32_t APB1RSTR;
    uint32_t IOPENR;
} RCC_RegDef_t;


// Macros para los dígitos del display
#define NUM_cero   0x3F
#define NUM_uno    0x06
#define NUM_dos    0x5B
#define NUM_tres   0x4F
#define NUM_cuatro  0x66
#define NUM_cinco   0x6D
#define NUM_seis   0x7D
#define NUM_siete  0x07
#define NUM_ocho   0x7F
#define NUM_nueve  0x67
#define LETRA_A    0x77
#define LETRA_B    0x7C
#define LETRA_C    0x39
#define LETRA_D    0x4E
#define LETRA_E    0x79
#define LETRA_F    0x71

#define KEY_A_INDEX 3
#define KEY_B_INDEX 7
#define KEY_C_INDEX 11
#define KEY_D_INDEX 15
#define KEY_F_INDEX 14
#define KEY_E_INDEX 12

void delay_ms(uint16_t n);
void actualizar_valores(void); // Elimina el argumento aquí
uint8_t decoder(uint8_t value_to_decode);
void decimal_decoder(uint8_t value);

uint8_t primer_digito_pantalla = 0xFF; // Primer dígito para mostrar
uint8_t segundo_digito_pantalla = 0xFF; // Segundo dígito para mostrar
uint8_t tercer_digito_pantalla = 0xFF; // Segundo dígito para mostrar
uint8_t cuarto_digito_pantalla = 0xFF; // Segundo dígito para mostrar
uint8_t quinto_digito_pantalla = 0xFF;
uint8_t valor_guardado = 0; // Variable para almacenar el valor mostrado en los displays
int valor_producto = 0;

const uint8_t key_to_display[] = {
	NUM_uno, NUM_dos, NUM_tres, LETRA_A, // Columna 1: 1, 2, 3, A
	NUM_cuatro, NUM_cinco, NUM_seis, LETRA_B, // Columna 2: 4, 5, 6, B
	NUM_siete, NUM_ocho, NUM_nueve, LETRA_C, // Columna 3: 7, 8, 9, C
	LETRA_E, NUM_cero, LETRA_F, LETRA_D // Columna 4: *, 0, #, D
};

uint8_t get_number_from_key(uint8_t key){
	switch (key){
		case 0: return 1;
		case 1: return 2;
		case 2: return 3;
		case 4: return 4;
		case 5: return 5;
		case 6: return 6;
		case 8: return 7;
		case 9: return 8;
		case 10: return 9;
		case 13: return 0;
		default: return 0;
	}
}

uint8_t get_key_from_number(uint8_t number){
	switch (number){
		case 1: return 0;
		case 2: return 1;
		case 3: return 2;
		case 4: return 4;
		case 5: return 5;
		case 6: return 6;
		case 7: return 8;
		case 8: return 9;
		case 9: return 10;
		case 0: return 13;
		default: return 13;
	}
}

int main(void)
{

    // Habilitar el reloj para GPIOC y GPIOB
    RCC->IOPENR |= (1 << 2) | (1 << 1) | (1 << 0);

    // Configurar los pines de GPIOC como salida para los displays (asumiendo que esto ya está hecho correctamente)
    uint32_t portC_masks = (0b01 << 8) | (0b01 << 10) | (0b01 << 12) | (0b01 << 14) | (0b01 << 16) | (0b01 << 18);
    GPIOC->MODER &= ~(0b11 << 8 | 0b11 << 10 | 0b11 << 12 | 0b11 << 14 | 0b11 << 16 | 0b11 << 18);
    GPIOC->MODER |= portC_masks;

    // Configurar los pines de GPIOB como salida para los segmentos de los displays
    GPIOB->MODER &= ~(0xFFFF); // Limpiar los modos de los primeros 8 pines (pines 0 a 7)
    uint32_t portB_display_masks = (0b01 << 0) | (0b01 << 2) | (0b01 << 4) | (0b01 << 6) |
                                   (0b01 << 8) | (0b01 << 10) | (0b01 << 12) | (0b01 << 14);
    GPIOB->MODER |= portB_display_masks; // Establecer los pines del display como salida

    // Configurar los pines de GPIOB como salida para controlar las columnas de la matriz del teclado
    GPIOB->MODER &= ~((0b11 << 24) | (0b11 << 26) | (0b11 << 28) | (0b11 << 30)); // Limpiar la configuración actual para los pines 12 a 15
    GPIOB->MODER |= ((0b01 << 24) | (0b01 << 26) | (0b01 << 28) | (0b01 << 30)); // Establecer los pines PB12 a PB15 como salidas

    // Configurar los pines del 16 al 22 de GPIOB con resistencias pull-up
    GPIOB->PUPDR &= ~((0b11 << 16) | (0b11 << 18) | (0b11 << 20) | (0b11 << 22)); // Limpiar configuración de pull-up/pull-down
    GPIOB->PUPDR |= ((0b01 << 16) | (0b01 << 18) | (0b01 << 20) | (0b01 << 22)); // Establecer pull-up para pines de entrada

    // Configurar los pines PB8 a PB11 como entrada (teclado matricial)
    GPIOB->MODER &= ~((0b11 << 16) | (0b11 << 18) | (0b11 << 20) | (0b11 << 22)); // Limpiar la configuración actual para los pines 8 a 11



    while (1)
    {
        for (int col = 0; col < 4; col++)
        {
            GPIOB->ODR = (0xF << 12); // Desactiva todas las columnas
            GPIOB->ODR &= ~(1 << (12 + col)); // Activa solo la columna actual

            delay_ms(10); // Deja tiempo para la estabilización3a}

            uint32_t rows = GPIOB->IDR & (0xF << 8); // Lee el estado de las filas

            for (int row = 0; row < 4; row++)
            {
                if (!(rows & (1 << (row + 8)))) // Detecta si alguna fila está activa (presionada)
                {
                    uint8_t key_number = col * 4 + row; // Ajusta el cálculo de key_number

                    if (key_number == KEY_D_INDEX) {
                        int sum_value = 0;
                        int weights[] = {1, 10, 100};  // Ponderaciones para unidades, decenas, centenas
                        uint8_t *digits[] = {&primer_digito_pantalla, &segundo_digito_pantalla, &tercer_digito_pantalla};  // Usando uint8_t* en lugar de int*

                        for (int i = 0; i < 3; i++) {
                            // Solo se suman los valores si el dígito no está en el estado 'no utilizado' (0xFF)
                            if (*digits[i] != 0xFF) {
                                sum_value += get_number_from_key(*digits[i]) * weights[i];
                            }
                        }

                        valor_guardado += (uint8_t)sum_value;  // Actualiza el valor guardado

                        // Resetea los valores de los dígitos en los displays
                        primer_digito_pantalla = segundo_digito_pantalla = tercer_digito_pantalla = cuarto_digito_pantalla = 0xFF;
                    }
                    else if (key_number == KEY_A_INDEX) {
                        if (valor_guardado != 0) {
                            int sum_value = 0;
                            int weights[] = {1, 10, 100};  // Ponderaciones para unidades, decenas, centenas
                            uint8_t *digits[] = {&primer_digito_pantalla, &segundo_digito_pantalla, &tercer_digito_pantalla};

                            for (int i = 0; i < 3; i++) {
                                if (*digits[i] != 0xFF) {
                                    sum_value += get_number_from_key(*digits[i]) * weights[i];
                                }
                            }

                            valor_guardado += (uint8_t)sum_value;

                            // Descomponiendo y actualizando los displays
                            for (int i = 0; i < 3; i++) {
                                int value = (valor_guardado / weights[i]) % 10;
                                *digits[i] = get_key_from_number(value);
                            }

                            valor_guardado = 0;
                        }
                    }

                    else if (key_number == KEY_B_INDEX)
                    {
                        // Llama a la función para decodificar el valor almacenado en los displays
                        if (valor_guardado != 0)
                        {


                        	int sum_value = 0;

                        	// Unidades
                        	int u_value = get_number_from_key(primer_digito_pantalla);
                        	// Decenas
                        	int d_value = get_number_from_key(segundo_digito_pantalla);
                        	// Centenas
                        	int c_value = get_number_from_key(tercer_digito_pantalla);
                        	// Millares
                        	//uint8_t m_value = get_number_from_key(cuarto_digito_pantalla);

                        	sum_value =  sum_value +  u_value + (d_value * 10) + (c_value * 100);

                        	valor_guardado = (uint8_t)(valor_guardado - sum_value);

                        	u_value = valor_guardado % 10;
                        	d_value = ((valor_guardado - u_value) % 100) / 10;
                        	c_value = ((valor_guardado - d_value - u_value) % 1000) / 100;

                        	primer_digito_pantalla =  get_key_from_number(u_value);
                        	segundo_digito_pantalla = get_key_from_number(d_value);
                        	tercer_digito_pantalla =  get_key_from_number(c_value);

                        	valor_guardado = 0;

                        }
                    }

                    else if (key_number == KEY_C_INDEX) {
                        if (valor_guardado != 0) {
                            int valor_producto = 0;
                            int weights[] = {1, 10, 100, 1000};  // Ponderaciones para unidades, decenas, centenas, millares
                            uint8_t *digits[] = {&primer_digito_pantalla, &segundo_digito_pantalla, &tercer_digito_pantalla, &cuarto_digito_pantalla};
                            int input_value = 0;

                            // Calcula input_value usando los dígitos actuales
                            for (int i = 0; i < 4; i++) {
                                if (*digits[i] != 0xFF) {  // Verifica si el dígito es válido
                                    input_value += get_number_from_key(*digits[i]) * weights[i];
                                }
                            }

                            // Calcula el producto
                            valor_producto = valor_guardado * input_value;

                            // Descompone el valor_producto y actualiza los displays
                            for (int i = 0; i < 4; i++) {
                                int value = (valor_producto / weights[i]) % 10;
                                *digits[i] = get_key_from_number(value);
                            }

                            valor_guardado = 0;  // Reinicia valor_guardado
                        }
                    }

                    else if (key_number == KEY_F_INDEX) {
                        // Inicializa el valor para la división
                        int div_value = 0;
                        int weights[] = {1, 10, 100};  // Ponderaciones para unidades, decenas, centenas
                        uint8_t *digits[] = {&primer_digito_pantalla, &segundo_digito_pantalla, &tercer_digito_pantalla};

                        // Calcula div_value usando los dígitos actuales
                        for (int i = 0; i < 3; i++) {
                            if (*digits[i] != 0xFF) {  // Verifica si el dígito es válido
                                div_value += get_number_from_key(*digits[i]) * weights[i];
                            }
                        }

                        // Realiza la división si es segura
                        if (valor_guardado != 0 && div_value != 0) {
                            valor_guardado = (uint8_t)(valor_guardado / div_value);

                            // Descompone el valor_guardado y actualiza los displays
                            for (int i = 0; i < 3; i++) {
                                int value = (valor_guardado / weights[i]) % 10;
                                *digits[i] = get_key_from_number(value);
                            }
                        }

                        valor_guardado = 0;  // Reinicia valor_guardado
                    }

                    else if (key_number == KEY_E_INDEX) {
                        // Verifica si valor_guardado y el nuevo valor no son cero para evitar división por cero
                        valor_guardado = 0;  // Reinicia valor_guardado

                        // Reinicia los dígitos de la pantalla
                        uint8_t *digits[] = {&primer_digito_pantalla, &segundo_digito_pantalla, &tercer_digito_pantalla, &cuarto_digito_pantalla};
                        for (int i = 0; i < 4; i++) {
                            *digits[i] = 0xFF;  // Establece cada dígito a 0xFF, indicando un estado de no utilizado o nulo
                        }
                    }

                    else{
                    	cuarto_digito_pantalla = tercer_digito_pantalla;

                    	tercer_digito_pantalla = segundo_digito_pantalla;
                        // Actualiza el segundo display con el valor del primer display
                        segundo_digito_pantalla = primer_digito_pantalla;
                        // Actualiza el primer display con el nuevo dígito
                        primer_digito_pantalla = key_number;
                    }

                    while (!(GPIOB->IDR & (1 << (row + 8)))); // Espera a que la tecla se suelte
                    break; // Sale del bucle de filas para evitar múltiples lecturas
                }
            }
            GPIOB->ODR |= (1 << (12 + col)); // Desactiva la columna actual antes de continuar
        }

        // Actualizar los valores de los displays
        actualizar_valores();
        valor_guardado = valor_guardado %1000;
    }


}

void delay_ms(uint16_t n) {
    for (uint32_t i = 0; i < n * 10; i++) {
        __asm__("NOP");
    }
}


void actualizar_valores(void) {

	if (cuarto_digito_pantalla != 0xFF) {
	    // Apaga todos los dígitos excepto el cuarto
	    int digit_pins[] = {5, 6, 8, 7}; // Los pines correspondientes a los dígitos 1, 2, 3 y 5
	    for (int i = 0; i < sizeof(digit_pins)/sizeof(digit_pins[0]); i++) {
	        GPIOC->BSRR = 1 << (digit_pins[i] + 16);
	    }

	    // Enciende el cuarto dígito
	    GPIOC->BSRR = 1 << 9;

	    // Apaga todos los segmentos y luego muestra el valor para el cuarto dígito
	    GPIOB->BSRR = 0xFF << 16; // Apaga todos los segmentos
	    GPIOB->BSRR = decoder(cuarto_digito_pantalla); // Muestra el cuarto dígito

	    delay_ms(5); // Retardo para la visualización
	}

	if (tercer_digito_pantalla != 0xFF) {
	    // Apaga los otros dígitos
	    int digit_pins[] = {5, 6}; // Los pines correspondientes a los dígitos 1 y 2
	    for (int i = 0; i < sizeof(digit_pins)/sizeof(digit_pins[0]); i++) {
	        GPIOC->BSRR = 1 << (digit_pins[i] + 16);
	    }

	    // Enciende el tercer dígito
	    GPIOC->BSRR = 1 << 8;

	    // Apaga todos los segmentos y luego muestra el valor para el tercer dígito
	    GPIOB->BSRR = 0xFF << 16; // Apaga todos los segmentos
	    GPIOB->BSRR = decoder(tercer_digito_pantalla); // Muestra el tercer dígito

	    delay_ms(5); // Retardo para la visualización
	}
    // Multiplexación para mostrar los números en los displays
	if (segundo_digito_pantalla != 0xFF) {
	    // Apaga los otros dígitos
	    int digit_pins[] = {5, 8}; // Los pines correspondientes a los dígitos 1 y 3
	    for (int i = 0; i < sizeof(digit_pins)/sizeof(digit_pins[0]); i++) {
	        GPIOC->BSRR = 1 << (digit_pins[i] + 16);
	    }

	    // Enciende el segundo dígito
	    GPIOC->BSRR = 1 << 6;

	    // Apaga todos los segmentos y luego muestra el valor para el segundo dígito
	    GPIOB->BSRR = 0xFF << 16; // Apaga todos los segmentos
	    GPIOB->BSRR = decoder(segundo_digito_pantalla); // Muestra el segundo dígito

	    delay_ms(5); // Retardo para la visualización
	}

	if (primer_digito_pantalla != 0xFF) {
	    // Apaga los otros dígitos
	    int digit_pins[] = {6, 8}; // Los pines correspondientes a los dígitos 2 y 3
	    for (int i = 0; i < sizeof(digit_pins)/sizeof(digit_pins[0]); i++) {
	        GPIOC->BSRR = 1 << (digit_pins[i] + 16);
	    }

	    // Enciende el primer dígito
	    GPIOC->BSRR = 1 << 5;

	    // Apaga todos los segmentos y luego muestra el valor para el primer dígito
	    GPIOB->BSRR = 0xFF << 16; // Apaga todos los segmentos
	    GPIOB->BSRR = decoder(primer_digito_pantalla); // Muestra el primer dígito

	    delay_ms(5); // Retardo para la visualización
	}

}

uint8_t decoder(uint8_t number) {
	uint8_t key = 0;
	switch (number){
		case 0: key = 0; break;
		case 1: key = 1; break;
		case 2: key = 2; break;
		case 4: key = 4; break;
		case 5: key = 5; break;
		case 6: key = 6; break;
		case 8: key = 8; break;
		case 9: key = 9; break;
		case 10: key = 10; break;
		case 13: key = 13; break;
		default: return 13;
	}
	return key_to_display[key];
}

void decimal_decoder(uint8_t value) {
    // Verifica si el valor almacenado es un dígito válido (excluyendo A, B, C, D, * y #)
    if (value < 10) {
        // Actualiza los displays con el valor almacenado
    	tercer_digito_pantalla = segundo_digito_pantalla;
        segundo_digito_pantalla = primer_digito_pantalla; // Mueve el primer dígito al segundo dígito
        primer_digito_pantalla = value; // Establece el primer dígito como el valor almacenado
    }
}

