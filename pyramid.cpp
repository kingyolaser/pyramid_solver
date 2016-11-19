#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/****************************************************************************/
#define LAYERS    6
#define WIDTH     7
#define STOCK_LEN 28

class Board{
public:
    enum{
        card_invalid=-1,
        card_empty=0,
    };

    int tableau[LAYERS+2][WIDTH+2];  //例：[1][1]の上(画面上は下)に[1][2], [2][2]がかぶってる
    int stock[STOCK_LEN+1];     //stock[0]が最初のカード。nullターミネート
    int stock_nowpos;
    int pile_card;
    int tesuu;
    
    void init();
    void print();
    
    bool isExposed(int layer, int x)const{return tableau[layer+1][x]== card_empty && tableau[layer+1][x+1]==card_empty;};
    bool isremovable(int layer, int x)const;
    
    int  c2i(char c);
    char i2c(int i){return " A234567890JQK*"[i];}
    void setTableau(int layer, int x, char val){tableau[layer][x]=c2i(val);}
    void setTableau_layerall(int layer, const char *);
}board;
/****************************************************************************/
void Board::init()
{
    memset( tableau, 0, sizeof(tableau) );
}
/****************************************************************************/
void Board::print()
{
    for(int layer=0; layer<LAYERS; layer++ ){
        for( int i=0; i<LAYERS-layer; i++){printf(" ");}
        for( int x=0; x<layer+1; x++ ){
            printf("%c ", i2c(tableau[layer][x]));
        }
        printf("\n");
    }
}
/****************************************************************************/
int Board::c2i(char c)
{
    const char table[] = "A234567890JQK";
    const char *pos = strchr( table, c);
    if( pos ){return pos-table+1;}
    if( c=='1' ){return 1;}
    if( c==' ' ){return card_empty;}
    return card_invalid;
}
/****************************************************************************/
void Board::setTableau_layerall(int layer, const char *data)
{
    assert(strlen(data)>=(size_t)layer);
    for(int i=0; i<layer+1; i++){
        tableau[layer][i] = c2i(data[i]);
    }
}

/****************************************************************************/
void read_cardseq(const char prompt[], int layer, int length)
{
    static char  buf[80];
    for(;;){
        printf("Input layer-3 sequence > ");
        fgets(buf, sizeof(buf), stdin);
        
        //length check
        if( strlen(buf)!=WIDTH+1 ){ continue; } //+1 means '\n'
        if( buf[WIDTH]!='\n' ){continue;}
        buf[WIDTH]='\0';
        
        //to Upper
        int i;
        for(int i=0; i<WIDTH; i++){
            buf[i] = toupper(buf[i]);
        }
        
        //check 数字 or AJQK
        for( i=0; i<WIDTH; i++){
            if( strchr("1234567890JQKA", buf[i])==NULL){
                break;
            }
        }
        if(i!=WIDTH){
        	continue;  //check NG
        }
        break; //input OK
    }
    board.setTableau_layerall(layer, buf);
}

/****************************************************************************/
void usage()
{
    exit(0);
}

/****************************************************************************/
int main()
{
    Board board;

    board.init();
    board.print();
    return 0;
}
/****************************************************************************/
void test()
{
}
