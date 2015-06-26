#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


/*
Palavras reservadas:
 		inicio, fim, leia, escreva.

Operadores: 
  		,(a virgula),+, -, (, ), ;, :=

Numeros (um ou mais digitos):
 		0,1,2,3,4,5,6,7,8,9
 
Identificadores: 
 		Formados por letras e numeros. Tamanho maximo: 32
   
Comentarios: 
 		Iniciados por -- e terminam no final da linha
     
Espaco em branco:
  		Um espaco em branco eh inserido no final de cada linha

 */


#define VALOR	1
#define SIMBOLO	2
#define TOKEN 	3

#define SIZE 1024  // Tamanho do buffer
static char token_buffer[SIZE];

// Armazena a linha que contem erro de sintaxe
int linha_erro;

// Armazena o valor de uma variavel na expressao
//int val = 0;

// Lista de id's
int idlist[100];
int c_idlist = 0;

// Declara uma variavel global do tipo arquivo
FILE *arq;

// Define tipos de tokens possiveis na linguagem
typedef enum token_types {
  BEGIN, END, READ, WRITE, ID, INTLITERAL,
  LPAREN, RPAREN, SEMICOLON, COMMA, ASSIGNOP,
  PLUSOP, MINUSOP, SCANEOF
} token;


// Declarando estrutura dos simbolos 
typedef struct nodo {
	int tipo;
	union {
		int valor;
		char s[400];
		token t;
	};
} nodo;

// Declarando tipo da tabela
typedef struct dados {
	char simbolo[500];
	int valor;
}dados;


// Cria a tabela
dados tabelaSimbolos[2000];

//Contador de simbolos da tabela
int contador_simbolos = 0;
char *aux;

// Limpa o Buffer
void limpaBuffer(){
	strcpy(token_buffer, "");
}


// Imprime mensagem de erro
void imprimeErro(){
	printf("\nOcorreu um erro na linha:\t%d", __LINE__);
	exit(EXIT_SUCCESS);
}


// Verifica se a 'string' eh uma palavra reservada ou um ID
token palavraReservada(){
	
	if(strcmp(token_buffer,"inicio") == 0) {
		//printf("BEGIN ");
					return BEGIN; 
	} else if(strcmp(token_buffer,"fim") == 0) {
		//printf("END ");
					return END;
	} else if(strcmp(token_buffer,"leia") == 0) {
		//printf("READ ");
					return READ;
	} else if(strcmp(token_buffer,"escreva") == 0) {
		//printf("WRITE ");
					return WRITE;
	} 
	
	//printf("ID "); 
	return ID;
		
}


// Pega o caractere lido do arquivo e concatena com a 'string' 
// que ja esta na variavel token_buffer
void concatenar_char(char c) {
    char aux[2];
    aux[0] = c;
    aux[1] = '\x0';
    strncat(token_buffer, aux, SIZE);
}


// Toda vez que o scanner for chamado, vai retornar um token
token scanner(){
	int arq_c, c;	
	limpaBuffer();

	if(feof(arq)){  // Se for final de arquivo
    //	printf("SCANEOF ");
		return SCANEOF;
	}
	
	// Enquanto o caractere que pegar no arquivo for diferente de fim de arquivo
	while((arq_c = fgetc(arq)) != EOF) {
    	
		if (isspace(arq_c)){  				// Se for espaço
      			continue; 					// Nao faca nada. Continue
      	} else if (isalpha(arq_c)){			// Se for um caractere
				concatenar_char(arq_c); 	// Concatena com o buffer
				
				// A função isalnum checa se eh um caracter alfanumerico
				// O for vai rodar enquanto o caractere for um alfanumerico
      			for(c = fgetc(arq); isalnum(c); c = fgetc(arq)){
						concatenar_char(c);
      			}
				// Quando um caractere eh lido atraves de uma funcao fgetc,
      			// o ponteiro avanca um caractere. Com a funcao ungetc, eh
      			// possivel retornar uma posicao no arquivo.
				ungetc(c, arq);
      			// Verifica se a palavra armazenada eh uma palavra reservada da linguagem
      			return palavraReservada();  
    	} else if (isdigit(arq_c)) {		// Se for um digito
      			concatenar_char(arq_c);		// Concatena na 'string'
      			// O for vai rodar enquanto o caractere for um digito, e o concatena
      			// na 'string' buffer
      			for(c = fgetc(arq); isdigit(c);	c = fgetc(arq))
							concatenar_char(c);
				// Quando o caractere nao for mais um digito, chamamos a funcao
				// ungetc para retornar o caractere lido
      			ungetc(c, arq);
      			//printf("INTLITERAL ");
      			return INTLITERAL;			  
    	} else if(arq_c == '(') {
    			// '('
    		//printf("LPAREN ");
					return LPAREN;
		} else if(arq_c == ')') {
				// ')'
			//printf("RPAREN ");
					return RPAREN;
		} else if(arq_c == ';') {
				// ';'
			//printf("SEMICOLON ");
					return SEMICOLON;
		} else if(arq_c == ',') {
				// ','
			//printf("COMMA ");
					return COMMA;
		} else if (arq_c == ':') {
				// ':'
				// Se for :, passa a procurar uma atibuicao, entao pega mais um caractere
      			c = fgetc(arq);
      			//Se for um igual, achou atribuicao ':='
      			if (c == '='){
      				//printf("ASSIGNOP ");
						return ASSIGNOP;
      			} else {
      			// Senao devolve o caractere
						ungetc(c, arq);
				// Imprime erro
						printf("\nOcorreu um erro na linha:\t%d", __LINE__);
      			}
      			// Se for um hifen, passa a procurar por mais um hifen
				// para resultar em um simbolo de comentario
    	} else if (arq_c == '-') {
    			// Pega mais um caractere e compara se  eh um hifen
      			c = fgetc(arq);
      			// Se for, achou um comentario
      			if (c == '-') {
						// Entao ignora toda a linha
						do{
	  						arq_c = fgetc(arq);
						} while (arq_c != '\n');
      			} else {
      			// Se nao for, devolve o caractere e retorna o token
				// equivalente a um sinal de menos	
						ungetc(c, arq);
						//printf("MINUSOP ");
						return MINUSOP;
      			}
    			// Se encontrar um sinal de mais, eh uma operacao de soma
    	} else if (arq_c == '+') {
    	      //printf("PLUSOP ");
			  return PLUSOP;
    	} else 
    			// Se passar por todas as outras comparacoes entao eh um erro
      			//imprimeErro();
      			printf("\nOcorreu um erro na linha:\t%d", __LINE__);
      			
	}
	
}


// Cria uma variavel estatica do tipo token para armazenar um token
// Define seu valor inicial como -1, para indicar que nao existe token
// armazenado na variavel e o -1 tambem sera usado para 'apagar' o token
static token aux_token = -1;


// Chama a funcao scanner que retorna um token que sera salvo
// na variavel auxiliar declarada acima (aux_token)
// Esta eh a variavel que sera retornada ela funcao
token prox_token() {
  	if(aux_token == -1){
    		token t = scanner();
    		aux_token = t;
  	}
  	return aux_token;
}


// A funcao match vai comparar o token que eh passado quando
// ela eh chamada e compara com o token armazenado na variavel
// auxiliar (aux_token)
void match(token t) {
  	token c = prox_token();		    // c recebe o retorno da funcao prox_token
  	if (t != c){					// Compara o valor recebido com o que foi passado ao chamar a funcao
    	//imprimeErro();		    	// Se for diferente, imprime mensagem de erro
    	exit(EXIT_FAILURE);			
  	} else {				
    	aux_token = -1;				// Se forem iguais, limpa aux_token atribuindo -1 
  	}
}


// Realiza uma busca por uma variavel na tabela
int buscaTabelaSimbolos(char *s) {
	int num; 
	for (num = 0; num < contador_simbolos; num++) {
		if (strncmp(s, tabelaSimbolos[num].simbolo, 300) == 0){
			// Caso encontre, retorna a posicao
			return num;
		}
	}
	// Se nao encontrar, retorna -1
	return -1;
}


// Busca na tabela o valor de um simbolo especifico
int valorSimbolo(char *s) {
	int i;
	i = buscaTabelaSimbolos(s);
	return tabelaSimbolos[i].valor;
}


// Grava o valor de uma variavel na tabela de simbolos
void gravaNovoValor(char *s, int n)
{
    int p;
    p = buscaTabelaSimbolos(s);
    tabelaSimbolos[p].valor = n;
}


// Adiciona um simbolo na tabela de simbolos
void adicionaSimbolo(char *s){
	if (buscaTabelaSimbolos(s) == -1) {
		tabelaSimbolos[contador_simbolos].valor = 0;
		strncpy(tabelaSimbolos[contador_simbolos].simbolo, s, 300);
		contador_simbolos++;
	}
}


// Le valor armazenado. Retorna valor ou simbolo
int leValor(nodo no) {
	int valor;

	switch (no.tipo) {
		case VALOR:
			valor = no.valor;
			break;
		case SIMBOLO:
			valor = valorSimbolo(no.s);
			break;
	}
	
	return valor;
}



int expression();


// Nao terminal: add_op
//				 pode ser MINUSOP ou PLUSOP 
//				 se nao for, imprime mensagem de erro
nodo add_op(){
	nodo no;
	token t = prox_token();
	if (t == MINUSOP || t == PLUSOP){
			no.tipo = TOKEN;
			no.t = t;
			match(t);
	} else {
			//imprimeErro();
			printf("\nOcorreu um erro na linha:\t%d", __LINE__);
	}
	return no;
} 


// Nao terminal: primary
//				 pode ser LPAREN, ID, INTLITERAL 
//				 se nao for, imprime mensagem de erro

int primary(){
	nodo no;
	int valor, idvalor;
	token t = prox_token();	
	switch(t){
		case LPAREN:
					match(LPAREN);							// Tenta casar o prox_token com LPAREN 
					valor = expression();						// Depois chama a funcao expression para tentar casar
					match(RPAREN);							// Para seguir as regras da linguagem tem que terminar
					break;									// casando com um RPAREN
		case ID:	
					
					valor = valorSimbolo(token_buffer);    	// Pega valor da variavel que esta no buffer (expressao)
					if(buscaTabelaSimbolos(token_buffer) == -1){
						adicionaSimbolo(token_buffer);
						scanf("%d", &idvalor);
						gravaNovoValor(token_buffer, idvalor);
					}
					match(ID);								// Tenta casar o prox_token com um ID
					break;
		case INTLITERAL:
					valor = atoi(token_buffer);
					
					match(INTLITERAL);						// Tenta casar o prox_token com um INTLITERAL
					break;
		default:
					//imprimeErro();						// Se nao for nenhum dos casos acima, imprime mensagem de erro
					printf("\nOcorreu um erro na linha:\t%d", __LINE__);
					break;
	}
	return valor;
}


// Nao terminal: expression
//				 a sintaxe eh <primary> {<add_op> <primary>}
int expression(){
	nodo atual, novo, op;
	int a, b;
	token t;
	atual.tipo = 1;
  	atual.valor = primary();			// Chama a funcao primary para tentar casar a primeira parte da sintaxe
	a = leValor(atual);			// Le um inteiro			
							    // O for vai rodar enquanto o prox_token for um PLUSOP ou MINUSOP 							
								// sempre pega o prox token antes rodar mais uma vez
  	for(t = prox_token(); t == PLUSOP || t == MINUSOP; t = prox_token()){
    		op = add_op();		// Chama a funcao add_op para tentar casar com a sintaxe
    		novo.tipo = 1;
    		novo.valor = primary();   // Chama a funcao primary para tentar casar com a sintaxe
    		
    		b = leValor(novo);	// Le um inteiro
    		switch(op.t) {
          				case PLUSOP:
             					a = a + b;		// Soma		
          						break;
          				case MINUSOP:
             					a = a - b;		// Subtrai
          						break;
        	}
  	}

  	//fprintf(stdout, "Resultado: %d\n", a);
	return a; 	
}


// Nao terminal: exp_list
//				 a sintaxe eh <expression> {,<exp_list>}
int expression_list(){
	int valor;
	valor = expression(); 					// Chama a funcao expression para tentar casar com a sintaxe
  	while (prox_token() == COMMA) {		// Verifica se o proximo token eh uma virgula para verificar
    		match(COMMA);				// se existem mais expressoes depois dela, casando com a 
    		valor = expression_list();			// sintaxe da exp_list
  	}									// Ter mais de uma expressa eh opcional na linguagem	
  	return valor;
}// Retorna Lista de inteiros


// Nao terminal: id_list
//				 a sintaxe eh <id> {,<id_list>}
nodo id_list(){
	nodo no;
	int valor;
	valor = primary(); 					// Chama a funcao primary para tentar casar com a sintaxe
  	while (prox_token() == COMMA) {		// Verifica se o proximo token eh uma virgula para verificar
    		match(COMMA);				// se existem mais id's depois dela, casando com a 
    		no = id_list();					// sintaxe da id_list
  	}
										// Ter mais de um id eh opcional na linguagem	
	return no;
}


// Nao terminal: statement
//				 podem existir tres casos, a sintaxe eh
//				 ID := <expression>
//				 leia (<id_list>)
//				 escreva (<exp_list>) 
void statement(){
	nodo no;
	int valor;
	token t = prox_token();
	
	switch(t){
		case ID:	
					no.tipo = SIMBOLO;		// Define como um simbolo
					strncpy(no.s, token_buffer, 300);
					adicionaSimbolo(no.s);	// Adiciona simbolo na tabela
					match(ID);				// Chama a funcao match, passando ID para tentar casar com a sintaxe
					match(ASSIGNOP);		// Chama a funcao match, passando ASSIGNOP para tentar casar com a sintaxe
					valor = expression();		// Chama a funcao expression para tentar casar com a sintaxe
					gravaNovoValor(no.s, valor);	// Grava valor do simbolo
					break;
		case READ:
					match(READ);			// Chama a funcao match, passando READ para tentar casar com a sintaxe
					match(LPAREN);			// Chama a funcao match, passando LPAREN para tentar casar com a sintaxe
					no = id_list();			// Chama a funcao id_list para tentar casar com a sintaxe
					match(RPAREN);			// Chama a funcao match, passando RPAREN para tentar casar com a sintaxe
					break;
		case WRITE:
					match(WRITE);			// Chama a funcao match, passando WRITE para tentar casar com a sintaxe
					match(LPAREN);			// Chama a funcao match, passando LPAREN para tentar casar com a sintaxe
					valor = expression_list();	// Chama a funcao exp_list para tentar casar com a sintaxe
					match(RPAREN);			// Chama a funcao match, passando RPAREN para tentar casar com a sintaxe
					printf("%d\n", valor);			// Imprime valor da variavel
					break;
					//imprimeErro();		// Se nao for nenhum dos casos acima, imprime mensagem de erro
		default:
					break;	
	}	
}


// Nao terminal: statement_list
//				 a sintaxe eh <statement> {;<statement_list>}
void statement_list(){
	statement(); 						// Chama a funcao statement para tentar casar com a sintaxe
	token teste = prox_token();
  	while (teste == SEMICOLON) {	// Verifica se o proximo token eh ponto e virgula para verificar
    		match(SEMICOLON);			// se existem mais listas de statement's depois dela, casando
    		statement_list();			// com a sintaxe da statement_list
  	}									// Ter mais de um statement list eh opcional na linguagem	
}


// Nao terminal: program
//				 a sintaxe eh inicio <statement_list> fim
void program(){
	match(BEGIN);						// Chama a funcao match, passando BEGIN para tentar casar com a sintaxe
	statement_list();					// Chama a funcao statement_list para tentar casar com a sintaxe
	match(END);							// Chama a funcao match, passando END para tentar casar com a sintaxe
}


// Nao terminal: system_goal
//				 a sintaxe eh <program> SCANEOF
void system_goal(){
	program();							// Chama a funcao program para tentar casar com a sintaxe
	match(SCANEOF);						// Chama a funcao match, passando SCANEOF para tentar casar com a sintaxe
}


int main(int argc, char *argv[]) {
  token t;
  arq = fopen("teste1.npd", "r");
  
  system_goal();
 
  fclose(arq);
  return EXIT_SUCCESS;
}
