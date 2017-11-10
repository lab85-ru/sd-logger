//------------------------------------------------------------------------------
// CONSOLE
// input string, edit string(backspace only), find cmd, run cmd.
// 
// v0.2a
//------------------------------------------------------------------------------

#include <stdint.h>
#include <string.h>

#include "console.h"
#include "console_cfg.h"
#include "console_cmd.h"

#if CONSOLE_DEBUG == 0
#define printf(...) (void)0
#else
#include <stdio.h> 		// for debug
#endif

#if CONSOLE_ALLFUNC_EN == 1
static uint32_t find_str_start_prev(uint8_t * s, const uint32_t index, const uint32_t s_size);
static uint32_t find_str_start_next(uint8_t * s, const uint32_t index, const uint32_t s_size);
static void console_get_str_hist( console_var_type * cn, view_hist_t v);
static void call_key_up( console_var_type * cn );
static void call_key_down( console_var_type * cn );
static void console_parse_esc_code( console_var_type * cn, uint8_t c );
static uint32_t console_find_cmd(const console_var_type *cn, const console_cmd_type consol_cmd_tab[], const uint32_t consol_cmd_tab_size);
static uint32_t console_find_arg( console_var_type *cn);
static void add_char_to_string( console_var_type *cn, uint8_t c );
static void del_char_from_string( console_var_type * cn);

//------------------------------------------------------------------------------
// run operation esc code
// KEY left
//------------------------------------------------------------------------------
static void call_key_left( console_var_type * cn )
{
    if (cn->cur_pos != strlen( (const char*)cn->in_str )){
		cn->cur_pos++;
        cn->call_print( ESC_CUR_LEFT );
    }
}

//------------------------------------------------------------------------------
// run operation esc code
// KEY rigth
//------------------------------------------------------------------------------
static void call_key_rigth( console_var_type * cn )
{
    if (cn->cur_pos != 0){
		cn->cur_pos--;
        cn->call_print( ESC_CUR_RIGTH );
    }
}

//------------------------------------------------------------------------------
// run operation esc code
// KEY UP
//------------------------------------------------------------------------------
static void call_key_up( console_var_type * cn )
{

    cn->call_print( ESC_CLR_STR ); // clear cur string
    cn->call_print("\r");
    cn->call_print( PRINT_PROMT );
    
    console_get_str_hist( cn, VIEW_PREV );
    if (cn->in_str[0] != '\0')
        cn->call_print( (const char *)cn->in_str );
}

//------------------------------------------------------------------------------
// run operation esc code
// KEY DOWN
//------------------------------------------------------------------------------
static void call_key_down( console_var_type * cn )
{

    cn->call_print( ESC_CLR_STR ); // clear cur string
    cn->call_print("\r");
    cn->call_print( PRINT_PROMT );

    console_get_str_hist( cn, VIEW_NEXT );
    if (cn->in_str[0] != '\0')
        cn->call_print( (const char *)cn->in_str );
}

//******************************************************************************
// tablicha ESC_CODE -> CALL
static const console_esc_code_type consol_esc_code_tab[] = {
{
    .esc = ESC_CUR_UP,          // KEY UP
    .call = &call_key_up,
},
{
    .esc = ESC_CUR_DOWN,        // KEY DOWN
    .call = &call_key_down,
},
{
    .esc = ESC_CUR_RIGTH,       // KEY rigth
    .call = &call_key_rigth,
},
{
    .esc = ESC_CUR_LEFT,        // KEY left
    .call = &call_key_left,
},

};

// razmer tablichi
static const uint32_t CONSOLE_ESC_CODE_TAB_SIZE = sizeof(consol_esc_code_tab) / sizeof(struct console_esc_code_st);
//******************************************************************************


//------------------------------------------------------------------------------
// Parse + run operation esc code
// 
//------------------------------------------------------------------------------
static void console_parse_esc_code( console_var_type * cn, uint8_t c )
{
	uint32_t len = 0;
	uint32_t len_esc_tab = 0;
	uint32_t i = 0;
	
	if (CONSOLE_DEBUG) printf("*** console_parse_esc_code ***\n\r");
	if (CONSOLE_DEBUG) printf("cn->esc_str_size = %d\n\r", cn->esc_str_size);
	
	// zahita ot perepolneniya
	if (cn->esc_str_size >= ESC_STR_MAX_SIZE){ // slihkom dlinnaya ESC posledovatelnost CLEAR + return
	    cn->esc_str_size = 0;
	    cn->mode = MODE_STR;
	    return;	
	}

	cn->esc_str[ cn->esc_str_size ] = c;
	cn->esc_str_size ++;

	cn->esc_str[ cn->esc_str_size ] = '\0'; // dobavili konec stroki

    if (CONSOLE_DEBUG){
		for(uint32_t j=0; j<ESC_STR_MAX_SIZE; j++) printf(" 0x%x", cn->esc_str[ j ]);
		printf("\r\n");
	}
	
    for (i=0; i<CONSOLE_ESC_CODE_TAB_SIZE; i++){
	    if (CONSOLE_DEBUG) printf("%s\n\r", consol_esc_code_tab[i].esc);
        if (CONSOLE_DEBUG) printf("%s\n\r", (const char*)cn->esc_str);

        len = strlen( (const char*)consol_esc_code_tab[i].esc );
        if (len > len_esc_tab){
            len_esc_tab = len;
	    }	       
                
	    if (strcmp( (const char*)consol_esc_code_tab[i].esc, (const char*)cn->esc_str) == 0){ // cmd naydena
	        if (CONSOLE_DEBUG) printf("*** ESC naydena ***\r\n");
	        if (consol_esc_code_tab[i].call != NULL) consol_esc_code_tab[i].call( cn );
     	    cn->esc_str_size = 0;  // clear ESC
	        cn->mode = MODE_STR;
	        return;
		}
	} // for -----------------------------------------------------------
	
	if ( strlen( (const char *)cn->esc_str ) >= len_esc_tab ){ // esc ne naydena !, len > esc to CLEAR
	    if (CONSOLE_DEBUG) printf("strlen( (const char *)cn->esc_str ) >= len_esc_tab\r\n");
   	    cn->esc_str_size = 0;
        cn->mode = MODE_STR;
	}
	
}

//------------------------------------------------------------------------------
// poisk nachala stroki v kolcevom buffere, VPERED na 1 stroku
// return - ukazatel mesto nachala stroki
// vid stroki : <str>0<str>0<str>0 00000000
//------------------------------------------------------------------------------
static uint32_t find_str_start_next(uint8_t * s, const uint32_t index, const uint32_t s_size)
{
	uint32_t i = index;
	uint32_t c_z = 0;    // counter 0, v stroki s zadi
	
	//printf("*** find_str_start_next ***\r\n");
	//printf("index = %d\r\n", index);

    if (*(s + i) == 0){// index ukazivaet na konech zapisi NET STROKI
        //printf("i = %d\r\n", i);
        return i;
    }

    // ishem konec stoki - \0
    if (*(s + i) != 0){ // t.e. index ukazivaet na nacnalo stroki, to ihem 0
		while(*(s + i) != '\0'){
		    //printf("*(s + %d) = %c\r\n", i, *(s + i));
		    i++;
		    i &= s_size - 1;
		}
	}
	
    // index ukazivaet na \0	
    // index + 1  = \0 ?
    c_z = 0;
    while(*(s + i) == '\0'){
		//printf("2 *(s + %d) = %c\r\n", i, *(s + i));
		c_z++;
		if (c_z >=2) break;  // sokrashaem chikl chot ne proverat ves massiv esli on pust
		i++;
		i &= s_size - 1;
	}
    
    //printf("c_z = %d\r\n", c_z);
    
    if (c_z >= 2){
		//printf("i = %d\r\n", i);
		return i;
    }

	//printf("i = %d\r\n", i);
	return i;
}

//------------------------------------------------------------------------------
// poisk nachala stroki v kolcevom buffere, NAZAD na 1 stroku ot position index
// return - ukazatel mesto nachala stroki
// vid stroki : <str>0<str>0<str>0 00000000
//------------------------------------------------------------------------------
static uint32_t find_str_start_prev(uint8_t * s, const uint32_t index, const uint32_t s_size)
{
	uint32_t i = index;
	uint32_t c_z = 0;    // counter 0, v stroki s zadi
	uint32_t p = 0;
	
	//printf("*** find_str_start_prev ***\r\n");
	//printf("index = %d\r\n", index);

    c_z = 0;
    if (*(s + i) != 0){ // t.e. index ukazivaet na nacnalo stroki, to ihem 0
		while(*(s + i) != '\0'){
		    //printf("*(s + %d) = %c\r\n", i, *(s + i));
		    c_z++;
		    i--;
		    i &= s_size - 1;
		}
	}
	
    // index ukazivaet na 0	
    c_z = 0;
    while(*(s + i) == '\0'){
		//printf("*(s + %d) = %c\r\n", i, *(s + i));
		c_z++;
		i--;
		i &= s_size - 1;
		if (c_z >=3) break;  // sokrashaem chikl chot ne proverat ves massiv esli on pust
	}
    
    //printf("\r\n c_z = %d\r\n", c_z);
    
    if (c_z >= 3){
		//printf("i = %d\r\n", index);
		return index;
	}
    
    // est stroka, ihem nachalo
    while(*(s + i) != '\0'){
		p = i; // zapomnili predidushee znachenie
		i--;
		i &= s_size - 1;
	}

    //printf("p = %d\r\n", p);
	//printf("i = %d\r\n", i);
	return p;
}


//------------------------------------------------------------------------------
// copy str_hist -> vhodnay stroka
// return:
// -1 - net stroki
// !=0 - position stroki
//------------------------------------------------------------------------------
static void console_get_str_hist( console_var_type * cn, view_hist_t v)
{
	uint32_t res;
	uint32_t i = 0;
	
	if (v == VIEW_NEXT)
        res = find_str_start_next(cn->hist, cn->hist_p, CONSOLE_HIST_STR_SIZE);
    else
        res = find_str_start_prev(cn->hist, cn->hist_p, CONSOLE_HIST_STR_SIZE);
    
	cn->hist_p = res;
	
	memset(cn->in_str, 0, CONSOLE_IN_STR_MAX_SIZE);
	
	while( cn->hist[ res ] != '\0'){
		if (i == CONSOLE_IN_STR_MAX_SIZE)  // zashita ot vihoda za predeli massiva
		    break;
	    cn->in_str[ i ] = cn->hist[ res ];
	    i++;
	    res++;
	    res &= CONSOLE_HIST_STR_SIZE - 1;
	}
	
	cn->in_str_size = strlen( (const char*)cn->in_str );
}

//------------------------------------------------------------------------------
// dobavlenie stroki v buffer istorii
// 
//------------------------------------------------------------------------------
static void console_add_hist( console_var_type * cn)
{
	uint32_t i,c;
	uint32_t len;
	
	if (CONSOLE_DEBUG) {
	    for (uint32_t j=0; j<CONSOLE_HIST_STR_SIZE; j++) printf(" 0x%x", cn->hist[ j ]);	
	}
	
	len = strlen((const char*)cn->in_str);
	if (len == 0) return;
	
	for (i=0; i<len; i++){
		cn->hist[ cn->hist_in ] = cn->in_str[ i ];
		cn->hist_in ++;
		cn->hist_in &= CONSOLE_HIST_STR_SIZE - 1; // perehod v nachalo buffera - RING
	}
	cn->hist[ cn->hist_in ] = '\0';               // dobavili v konec stroki NULL
	cn->hist_in ++;
	cn->hist_in &= CONSOLE_HIST_STR_SIZE - 1;     // perehod v nachalo buffera - RING
	
	// esli pishem powerh stroki TO zatiraem stroku do 0, chtobi ubrat iz istrorii nepolnie stroki
	c = cn->hist_in + 1;
	if (cn->hist[ c ] != '\0'){
		while (cn->hist[ c ] != '\0'){
			cn->hist[ c ] = '\0';
			c++;
			c &= CONSOLE_HIST_STR_SIZE - 1;
		}
	}
	
}
#endif // CONSOLE_ALLFUNC_EN ---------------------------------------------------

//------------------------------------------------------------------------------
// Poisk cmd v spiske i vipolnenie cmd
// return = 0 cmd found
//------------------------------------------------------------------------------
static uint32_t console_find_cmd(const console_var_type *cn, const console_cmd_type consol_cmd_tab[], const uint32_t consol_cmd_tab_size)
{
    if (CONSOLE_DEBUG) printf("\n*** console_find_cmd ***\n");
    
    
    for (uint32_t i=0; i< consol_cmd_tab_size; i++){
	 
	    if (CONSOLE_DEBUG) printf("%s\n\r", consol_cmd_tab[i].cmd);
        if (CONSOLE_DEBUG) printf("%s\n\r", (const char*)cn->argv[0]);

	    if (strcmp( (const char*)consol_cmd_tab[i].cmd, (const char*)cn->argv[0]) == 0){ // cmd naydena
	        if (CONSOLE_DEBUG) printf("*** cmd naydena ***");
	        if (consol_cmd_tab[i].call != NULL) consol_cmd_tab[i].call( cn );
	        return 0;
		}
	}

    if (CONSOLE_DEBUG) printf("*** cmd NENAYDENA naydena ***");
    return 1;
}


//------------------------------------------------------------------------------
// razbor stroki na argumenti
// return: kolichestvo argumentov
//------------------------------------------------------------------------------
static uint32_t console_find_arg( console_var_type *cn)
{
    uint32_t i = 0;
    uint32_t n = 0;  // kolichestvo simvolov v stroke
    uint32_t si = 0; // nachalo parametrea v vhodnoy stroke
    uint8_t c;
    uint32_t len = 0; // dlinna vhodnoy stroki
    
    cn->argc = 0;

	if (CONSOLE_DEBUG) {
        printf("\r\n");
        for(int j=0;j<CONSOLE_IN_STR_MAX_SIZE;j++){
            printf("0x%02X ", cn->in_str[j]);
        }
        printf("\r\n");
	}
    
    if (strlen((char*)cn->in_str) == 0){
        cn->argc = 0;
        for (i=0; i<CONSOLE_ARGC_SIZE; i++){
            cn->argv[i] = 0; 
        }
        if (CONSOLE_DEBUG) printf("cn->argc = %d\n", cn->argc);
        return cn->argc;
    }

    len = strlen((char*)cn->in_str);
    
    for (i=0; i<len; i++){
        if (n == 0) si = i; // zapomnili adres nachala rapametra
        
        c = cn->in_str[ i ];
        
        if (c !=  ' ' && c !=  '\r' && c !=  '\n'){
            n++; // kolilichestvo simvolov v tekushem parametre
        }else{
            cn->in_str[i] = '\0'; // za mesto porobelov dobavlaem konic stroki \0
            if (n > 0){
                cn->argv[ cn->argc ] = &cn->in_str[ si ];

				if (CONSOLE_DEBUG) printf("cn->argv[ cn->argc ]= %s\r\n", (char *)cn->argv[ cn->argc ]);
				if (CONSOLE_DEBUG) printf("cn->argc = %d\r\n", cn->argc );

                cn->argc++;
                n = 0;
            }
        }
    }// for ------------------------------------------------------------
    
    if (n > 0 ){
        cn->argv[ cn->argc ] = &cn->in_str[ si ];

    	if (CONSOLE_DEBUG) printf("cn->argv[ cn->argc ]= %s\r\n", (char *)cn->argv[ cn->argc ]);
		if (CONSOLE_DEBUG) printf("cn->argc = %d\r\n", cn->argc );

        cn->argc++;
    }
    
    return cn->argc;
}

//------------------------------------------------------------------------------
// Dobavlenie char to string
// 
//------------------------------------------------------------------------------
static void add_char_to_string( console_var_type *cn, uint8_t c )
{
	uint32_t p = 0;
	uint8_t x,cx;
	uint32_t i;
	char str_echo[2];
	
    if (cn->cur_pos != 0){  // dobavlenie simvola v seredinu slova
	    // delaem perestanovku simvolov
	    p = strlen( (const char *)cn->in_str ) - cn->cur_pos;
	    
	    i = 0;
	    x = cn->in_str[ p + i ]; // vzali char s pos kuda budem zapisivat new char
	    cn->in_str[ p + i ] = c; // zapisali char v pos
	    // delaem sdvig simvolov
	    
	    i++;
	    while( (cx = cn->in_str[ p + i ]) != '\0'){
			cn->in_str[ p + i ] = x;
			x = cx;
			i++;
		}
	    cn->in_str[ p + i ] = x; // zapisali posledniya char
	    i++;
	    cn->in_str[ p + i ] = '\0';
	    
        cn->call_print( "\r" );
        cn->call_print( PRINT_PROMT );
        cn->call_print( (const char *)cn->in_str );
        for (i=0; i<cn->cur_pos; i++) cn->call_print( ESC_CUR_LEFT );
        
	    return;
	}
	
	// dobavlenie simvola v konec slova
    cn->in_str[ cn->in_str_size ] = c;
    cn->in_str_size++;
    str_echo[0] = c;
    str_echo[1] = 0;
    cn->call_print( str_echo ); // echo
	
}

//------------------------------------------------------------------------------
// udalaem simvol s proizvolnoy posichii in_str
//------------------------------------------------------------------------------
static void del_char_from_string( console_var_type * cn)
{
	uint8_t p,cx;
	uint32_t i;

    if (cn->in_str_size == 0){
	    return;
	}
	
	if (cn->cur_pos == 0){ // udalaem posledniy char v stroke
		cn->call_print("\r");
		cn->call_print( PRINT_PROMT );
		if (cn->in_str_size != 0){
			cn->in_str_size--;
			cn->in_str[ cn->in_str_size ] = '\0';
			
			cn->call_print( ESC_CLR_STR );
            cn->call_print( "\r" );
            cn->call_print( PRINT_PROMT );
            cn->call_print( (const char *)cn->in_str );
		}
		return;
	}// if ----------------------------------------
	
	// udalaem char v seredine stroki
	
    // delaem perestanovku simvolov
    p = strlen( (const char *)cn->in_str ) - cn->cur_pos - 1;
	    
    i = 0;

    // delaem sdvig simvolov <-
    while( (cx = cn->in_str[ p + i + 1 ]) != '\0'){
		cn->in_str[ p + i ] = cx;
		i++;
	}
    cn->in_str[ p + i ] = '\0';
	    
	cn->call_print( ESC_CLR_STR );
    cn->call_print( "\r" );
    cn->call_print( PRINT_PROMT );
    cn->call_print( (const char *)cn->in_str );

    for (i=0; i<cn->cur_pos; i++) cn->call_print( ESC_CUR_LEFT );
        
    return;
}


//------------------------------------------------------------------------------
// general CALL - CONSOLE
// 
//------------------------------------------------------------------------------
void console (console_var_type * cn, uint8_t c)
{
    int32_t res;
    
    if (CONSOLE_DEBUG) printf("GET char code = 0x%X\n", c);
    if (CONSOLE_DEBUG) printf("MODE = 0x%X\n", cn->mode );
    
#if CONSOLE_ALLFUNC_EN == 1

    if (c == CODE_ESC || cn->mode == MODE_ESC){
		if (CONSOLE_DEBUG) printf("mode = ESC\n\r");
		cn->mode = MODE_ESC;
        console_parse_esc_code( cn, c );
    } else \

#endif    
  
    if (c == CODE_BS || c == CODE_BS2){
		del_char_from_string( cn );
    } else if (c == CODE_CR || c == CODE_LF){

#if CONSOLE_ALLFUNC_EN == 1
        console_add_hist(cn);            // dobavlaem stroku v istoriyu
        cn->hist_p = cn->hist_in;        // sbross promotok Key Up-Down v nachanlo
        cn->cur_pos = 0;                 // cursor position = 0
#endif
        res = console_find_arg( cn );
        if (res){
            res = console_find_cmd( cn, consol_cmd_tab, CONSOLE_CMD_TAB_SIZE);
            if (res) cn->call_print( PRINT_CMD_NOT_FOUND );
        }
        memset(cn->in_str, 0, CONSOLE_IN_STR_MAX_SIZE); // storoka obrabotana teper chistim buffer
        cn->in_str_size = 0;
        cn->call_print( "\r\n" );
        cn->call_print( PRINT_PROMT );
    } else if (cn->in_str_size < CONSOLE_IN_STR_MAX_SIZE - 1){ // -1 t.k. posledniy = NULL
        add_char_to_string( cn, c);

    }

}

//------------------------------------------------------------------------------
// init console for current CN !!!
// cn - mohet bit neskolko
//------------------------------------------------------------------------------
void console_init (console_var_type * cn)
{
    uint32_t i;
    
    cn->argc = 0;                                    // kolichestvo argumentov
//    uint8_t* argv[ CONSOLE_ARGC_SIZE ];            // ukazatel na argumenti
    memset(cn->in_str, 0, CONSOLE_IN_STR_MAX_SIZE ); // buffer dla hraneniya vhodnoy stroki
    cn->in_str_size = 0;                             // kolichestvo simvolov vo shodnoy stroke
    cn->mode = MODE_STR;                             // mode = input string (NO ESC CODE)
    cn->esc_str_size = 0;
    cn->hist_in = 0;
    cn->cur_pos = 0;
    for (i=0; i<CONSOLE_HIST_STR_SIZE; i++) cn->hist[ i ] = '\0';
}
