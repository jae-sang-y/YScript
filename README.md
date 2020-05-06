# YScript

# YJS(YScript is Joyful Script)
## YJS 000 전체 구조
+ Yscript는 렉서, 로직빌더, 익스프레션 빌더, 어셈블러, 익스큐터로 구현된다.
+ 렉서에서는 평문코드를 토큰들로 변환한다.
+ 로직빌더에서는 토큰들을 통해 function, if, else, while 같은 제어구문을 분석하여 Tree로 변환한다.
+ 익스프레션 빌더에서는 토큰들을 통해 1줄로 나타낼수 있는 코드를 토큰 우선수위에 따라 Tree로 변환한다.
+ 어셈블러에서는 Tree를 통해 이를 Bytecodes(String들의 배열)로 변환한다.
+ 익스큐터에서는 Bytecodes를 통해 실행을 한다.
## YJS 001 YObject의 정의
+ String은 UTF8의 uint8_t의 배열을 의미한다.
+ YObject는 YScript내의 정보 저장에서 원소이다.
+ YObject는 Type, Data, Attributes, const 여부를 가지고 있다.
+ YObject는 YPtr을 통해 정보의 생성과 소멸을 관리한다.
+ Type는 YPtr 이며, Data는 void*, Attribute는 Key<String>:Value<YPtr> 형태이고.
+ is_const는 bool 형태이다.
## YJS 002 Type와 Instance의 관계와 Attribute
+ type를 통해 YObject를 만들수 있고 이때 만들어진 결과물을 Instance라고 한다.
+ 이때 type의 Attributes는 Instance도 가지고 이를 상속이라한다.
+ Instacne에 같은 Key를 가진 Attribute가 있다면 Type의 Attribute는 사용하지 않으며 이를 Overloading이라고 한다.
## YJS 003 Built-in types의 정의
+ type는 스스로를 Type로 가지는 type 이다.
+ null는 Data를 가지지 않는 type이다.
+ bool는 Data를 bool 형태로 가지는 type이다.
+ str는 Data를 String 형태로 가지는 type이다.
+ i32는 Data를 int32_t 형태로 가지는 type이다.
+ f32는 Data를 float 형태로 가지는 type이다.
+ list는 Data를 vector<YPtr> 형태로 가지는 type이다.
+ dict는 Data를 Key<YPtr>:Value<YPtr> 형태로 가지는 type이다.
## YJS 004 Built-in적 행동(생성, Operator)에서 호출되는 함수의 이름들
모든 Type는 이러한 함수를 Attributes에 가지고 있을수 있다.
+ __new__: Data를 메모리에 할당하고 Instance를 만든다.
+ __init__:  Instance 에 대해 Type가 Attributes를 처리해준다.
+ __gt__: 두 Instances 간의 ‘<’ 연산을 Type가 처리해준다.
+ __ge__: 두 Instances 간의 ‘<=’ 연산을 Type가 처리해준다.
+ __lt__: 두 Instances 간의 ‘>’ 연산을 Type가 처리해준다.
+ __le__: 두 Instances 간의 ‘>=’ 연산을 Type가 처리해준다.
+ __eq__: 두 Instances 간의 ‘==’ 연산을 Type가 처리해준다.
+ __ne__: 두 Instances 간의 ‘!=’ 연산을 Type가 처리해준다.
+ __add__: 두 Instances 간의 ‘+’ 연산을 Type가 처리해준다.
+ __sub__: 두 Instances 간의 ‘-’ 연산을 Type가 처리해준다.
+ __div__: 두 Instances 간의 ‘/’ 연산을 Type가 처리해준다.
+ __mod__: 두 Instances 간의 ‘%’ 연산을 Type가 처리해준다.
+ __mul__: 두 Instances 간의 ‘*’ 연산을 Type가 처리해준다.
+ __pow__: 두 Instances 간의 ‘**’ 연산을 Type가 처리해준다.
+ __and__: 두 Instances 간의 ‘&&’ 연산을 Type가 처리해준다.
+ __xor__: 두 Instances 간의 ‘^^’ 연산을 Type가 처리해준다.
+ __or__: 두 Instances 간의 ‘||’ 연산을 Type가 처리해준다.
+ __repr__: Instance 를 Type가 String으로 표현해준다.(표현된것을 다시 Instance로 변환할 수 있음은 보장하지 않는다.)
## YJS 005 Function과 Method의 정의
+ Function은 0개 이상의 인수를 가진다.
+ 그러나 Method는 1개(self<YPtr>) 이상의 인수를 가진다.
+ Function과 Method는 인수를 Keyword<String>와 함께 보낼수 있고 이러한 인수를
+ kwargs(Keywords Arguments)라고 하고 그렇지 않은 인수를 args(Normal Arguments)라고 한다.
