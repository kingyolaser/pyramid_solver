#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/****************************************************************************/
#define LAYERS    7
#define WIDTH     7
#define STOCK_LEN 24

class Pos{
public:
    int layer;
    int x;
    Pos():layer(0),x(0){}
    Pos(int layer_, int x_):layer(layer_),x(x_){}
    
    static const Pos stock1;
    static const Pos stock2;
    static const Pos nopos;
};
const Pos Pos::stock1(-1,-1);
const Pos Pos::stock2(-2,-2);
const Pos Pos::nopos(0,0);

class Move{
public:
    Move(){}
    Move(const Pos &p1_, const Pos &p2_):p1(p1_),p2(p2_){}
    Pos p1,p2;
};

class Board{
public:
    enum{
        card_invalid=-1,
        card_empty=0,
    };

    int tableau[LAYERS+2][WIDTH+2];  //例：[1][1]の上(画面上は下)に[1][2], [2][2]がかぶってる
    int stock[STOCK_LEN+1];     //stock[0]が最初のカード。末尾にemptyをつける。
    int stock_len;
    int stock_nowpos;
    int pile_card;
    int tesuu;
    
    void init();
    void init(int argc, const char *argv[]);
    void print();
    
    bool isComplete(){return tableau[1][1]==card_empty;}
    bool isExposed(int layer, int x)const{return tableau[layer+1][x]== card_empty && tableau[layer+1][x+1]==card_empty;};
    bool isremovable(int layer, int x)const;
    void search_candidate(Move candidate[16], int *num);
    
    void remove(int layer1, int x1, int layer2, int x2);
    
    int  c2i(char c);
    char i2c(int i){return " A234567890JQK*"[i];}
    void setTableau(int layer, int x, char val){tableau[layer][x]=c2i(val);}
    void setTableau_layerall(int layer, const char *);
}board;
/****************************************************************************/
void Board::init()
{
    memset( tableau, 0, sizeof(tableau) );
    memset( stock,   0, sizeof(stock)   );
    stock_len = stock_nowpos = pile_card = tesuu = 0;
}
/****************************************************************************/
void Board::init(int argc, const char *argv[])
{
    init();
    if( argc!= 1+LAYERS+1 ){
        fprintf(stderr, "cmd line option num err.\n");
    }

    //場のデータ格納
    for( int layer=1; layer<=LAYERS; layer++ ){
        assert(strlen(argv[layer])==(size_t)layer);
        for( int x=1; x<=layer; x++){
            tableau[layer][x] = c2i(argv[layer][x-1]);
        }
    }
    
    //Stockデータ格納
    assert(strlen(argv[LAYERS+1])==STOCK_LEN);
    for( int i=0; i<STOCK_LEN; i++){
        stock[i]=c2i(argv[LAYERS+1][i]);
    }
    stock_len = STOCK_LEN;
}

/****************************************************************************/
void Board::print()
{
    printf("\n");
    for(int layer=1; layer<=LAYERS; layer++ ){
        for( int i=0; i<LAYERS-layer; i++){printf(" ");}
        for( int x=1; x<=layer; x++ ){
            printf("%c ", i2c(tableau[layer][x]));
        }
        printf("\n");
    }

    printf("stock:\n");
    for( int i=0; i<stock_len; i++ ){
        printf("%c", i2c(stock[i]));
    }
    printf("\n");
    for( int i=0; i<stock_nowpos; i++){printf(" ");}printf("^^\n");
}
/****************************************************************************/
void Board::search_candidate(Move candidate[16], int *num)
{
    //exposedなcardを列挙
    Pos exposed[7];
    int expo_num = 0;
    for( int x=1; x<=WIDTH; x++){
        for( int layer=LAYERS; layer>=1; layer--){
            if( tableau[layer][x]==card_empty ){continue;}
            if( isExposed(layer,x) ){
                assert(expo_num<=7);
                exposed[expo_num] = Pos(layer,x);
                expo_num++;
            }
            //empty以外なので、どっちにしろもう上を調べる必要なし
            break;
        }
    }

//#ifdef DEBUG
    printf("exposed card: ");
    for( int i=0; i<expo_num; i++){
        printf("%d%d:", exposed[i].layer, exposed[i].x);
    }
    printf("\n");
//#endif
    
    //足して13になる組を検索
    *num = 0;
    for( int i=0; i<expo_num; i++ ){
        for( int j=i+1; j<expo_num; j++ ){
            if( tableau[exposed[i].layer][exposed[i].x]
              + tableau[exposed[j].layer][exposed[j].x] == 13 ){
                assert(*num<16);
                candidate[*num] = Move(exposed[i],exposed[j]);
                (*num)++;
            }
        }
    }

    //ストックエリアとの組み合わせを検索
    for( int i=0; i<expo_num; i++ ){
        if( tableau[exposed[i].layer][exposed[i].x]
                +stock[stock_nowpos] == 13 ){
            candidate[*num] = Move(exposed[i], Pos::stock1);
            (*num)++;
        }
    }

    for( int i=0; i<expo_num; i++ ){
        if( tableau[exposed[i].layer][exposed[i].x]
                +stock[stock_nowpos+1] == 13 ){
            candidate[*num] = Move(exposed[i], Pos::stock2);
            (*num)++;
        }
    }

//#ifdef DEBUG
    printf("candidate: ");
    for( int i=0; i<*num; i++){
        printf("%d%dx%d%d:", candidate[i].p1.layer,
                             candidate[i].p1.x,
                             candidate[i].p2.layer,
                             candidate[i].p2.x);
    }
//#endif

}

/****************************************************************************/
void Board::remove(int layer1, int x1, int layer2, int x2)
{
    if(tableau[layer1][x1]==13){
        tableau[layer1][x1] = card_empty;
    }
}

/****************************************************************************/
int Board::c2i(char c)
{
    c = toupper(c);
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
void solve(Board &board)
{
    if( board.isComplete() ){
        board.print();
        printf("Congraturation!!\n");
        exit(0);
    }

    Move candidate[16];
    int  num;
    board.search_candidate(candidate, &num);
}

/****************************************************************************/
int test();
int main(int argc, const char *argv[])
{
#ifdef TEST
    if(argc>=2 && strcmp(argv[1],"--test")==0 ){
        return test();
    }
#endif

    Board board;

    board.init(argc,argv);
    board.print();
    solve(board);
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
    const char *testdata[]={"hoge", "7","3j","620","280j","4j0ak","7k5q58","12k8462",
                     "4q17q0jkq716593394863599"};
    pBoard->init(9, testdata);
    pBoard->print();
    pBoard->remove(7,3,0,0);
    pBoard->print();
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
