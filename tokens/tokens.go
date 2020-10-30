package tokens

type TokenID int

const (
	PACKAGE  TokenID = iota //0
	MODE                    //1
	VAR                     //2
	IDENT                   //3
	EQUAL                   //4
	INT                     //5
	FLOAT                   //6
	BYTE                    //7
	STRING                  //8
	FUNC                    //9
	BRACKETL                //10
	BRACKETR                //11
	ROCKET                  //12
	BRACEL                  //13
	RETURN                  //14
	BRACER                  //15
)

var tokens = []tokInp{
	{"package", PACKAGE},            //0
	{"mode", MODE},                  //1
	{"var", VAR},                    //2
	{"=", EQUAL},                    //3
	{`\d+`, INT},                    //4
	{`\d+\.\d+`, FLOAT},             //5
	{`\d`, BYTE},                    //6
	{`".*"`, STRING},                //7
	{"f", FUNC},                     //8
	{`\(`, BRACKETL},                //9
	{`\)`, BRACKETR},                //10
	{"->", ROCKET},                  //11
	{"{", BRACEL},                   //12
	{"}", BRACER},                   //13
	{"return", RETURN},              //14
	{"[a-zA-Z][a-zA-Z0-9]*", IDENT}, //15
}
