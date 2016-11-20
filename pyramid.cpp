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
int test();
int main(int argc, char *argv[])
{
#ifdef TEST
    if(argc>=2 && strcmp(argv[1],"--test")==0 ){
        return test();
    }
#endif

    Board board;

    board.init();
    board.print();
    return 0;
}
/****************************************************************************/
#ifdef TEST
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

class FunctionTest : public CPPUNIT_NS::TestFixture{
    //テストクラス
    CPPUNIT_TEST_SUITE(FunctionTest);//登録のスタート
    CPPUNIT_TEST(test_test);
    CPPUNIT_TEST_SUITE_END();//登録の終了

protected:
    Board *pBoard;
    void test_test();

public:
    void setUp();
    void testDown();
};

/****************************************************************************/
CPPUNIT_TEST_SUITE_REGISTRATION(FunctionTest);

//テスト起動時に実行
void FunctionTest::setUp(){
    pBoard = new Board();
}

//テスト終了時に実行
void FunctionTest::testDown(){
    delete pBoard;
}
/****************************************************************************/
void FunctionTest::test_test()
{

}
/****************************************************************************/
int test()
{
    CPPUNIT_NS::TestResult controller;

    //結果収集
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);

    //途中結果の収集
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener(&progress);

    //テストを走らせる。テストを入れて走る
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    runner.run(controller);

    //結果を標準出力にする。
    CPPUNIT_NS::CompilerOutputter outputter(&result,CPPUNIT_NS::stdCOut());
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}
#endif //TEST
