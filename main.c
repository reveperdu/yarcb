#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define PH '-' // placeholder
#define RDELIM '|'
#define DEFQUITMSG "Goodbye"
#define DEFEMPTMSG "Did you say anything"
#define DEFUNAME "You"
#define DEFCNAME "Bot"
enum rule_sections{ptrn,resp,cond,actn};
typedef struct ruleset
{
    char ***r;//rules [m][4][n]
    int n; // number of rules
    int *w;//weight
} ruleset;
typedef struct varlist{
    char var[100][100];
    char name[100][10];
    int n;
}varlist;
typedef struct list{
    char **str;
    int n;
}list;
//match result
typedef struct mrslt{
int matched;//flag
list substr;
}mrslt;
/*alloc space for ruleset on heap*/
ruleset init_rset(int m,int n){
ruleset newrs;
int i,j;
size_t psize=sizeof(char*);
size_t csize=sizeof(char);
newrs.n=0;
newrs.r=malloc(m*psize);
for(i=0;i<m;i++){
newrs.r[i]=malloc(4*psize);
}
newrs.w=malloc(m*sizeof(int));
for(j=0;j<4;j++){
for(i=0;i<m;i++)
newrs.r[i][j]=malloc(n*csize);
}
return newrs;
}
/*the structure of var array is defined here:
$1-$9 input vars, $a-$z custom vars,
named vars are defined in another function*/
varlist init_var()
{
    varlist vlist;
    int i;
    for(i=1;i<10;i++){
        sprintf(vlist.name[i],"%d",i);
    }
    for(i=0;i<26;i++){
        sprintf(vlist.name[i+10],"%c",i+'a');
    }
    vlist.n=36;
    return vlist;
}
void lfree(list l){
if(l.n>0){
int i;
for(i=0;i<l.n;i++)free(l.str[i]);
free(l.str);}
}
void putlist(list l){
    int i;
    for(i=0;i<l.n;i++)puts(l.str[i]);
}
void trimn(char *s){
    if (s[strlen(s)-1] == '\n')
{
    s[strlen(s)-1] = '\0';
}
}
//might be useful if using English
void lower(char *s, int all){
int i;
int n=all?strlen(s):1;
int d='a'-'A';
for(i=0;i<n;i++){
    if(s[i]>='A'&&s[i]<='Z')
    s[i]+=d;
}
}
/*substitute str1 in pattern with str2*/
char *sub(char *pattern, char *str1, char *str2)
{
    int i = 0;
    int plen = strlen(pattern);
    int c = 0;
    char *temp=str1;
    while ((temp = strstr(temp, pattern)) != NULL)
    {
        c++;
        temp += plen;
    }
    int msize = strlen(str1) + c * (strlen(str2) - plen) + 1;
    char *strbuf = malloc(msize);
    int l;
    if (c > 0)
    {
        char *current = strstr(str1, pattern);
        l = current - str1;
        strncpy(strbuf, str1, l);
        strbuf[l] = '\0';
        while ((temp = strstr(current, pattern)) != NULL)
        {
            l = temp - current;
            strncat(strbuf, current, l);
            strcat(strbuf, str2);
            current = temp + plen;
        }
        strcat(strbuf, current);
    }
    else
    {
        strcpy(strbuf, str1);
    }
    return strbuf;
}
list split(char *str, char delim)
{
    char sdelim[2]={delim,'\0'};
    char *sbuf = malloc(strlen(str)+1);
    char **sarr = malloc(strlen(str) * sizeof(char *));
    strcpy(sbuf, str);
    sarr[0] = malloc(strlen(sbuf)+1);
    sarr[0] = strtok(sbuf, sdelim);
    if (sbuf != NULL)
    {
        int i = 0;
        while (sbuf != NULL)
        {
            sarr[i] = malloc(strlen(sbuf)+1);
            strcpy(sarr[i], sbuf);
            sbuf = strtok(NULL, sdelim);
            i++;
        }
        /*if first char is delim,
        strtok will ignore it*/
        if (sarr[0][0] == delim)
        {
            char *sbuf1 = malloc(strlen(sarr[0]) + 1);
            strcpy(sbuf1, sarr[0]);
            strcpy(sarr[0], sbuf1 + 1);
            free(sbuf1);
        }
        list l;
        l.str=sarr;
        l.n = i;
        free(sbuf);
        return l;
    }
    else{
    char *emsg="Cannot segment %s, which does not contain %c";
    fprintf(stderr,emsg,str,delim);
    }
}
/*match str against n patterns and
return , extracted substrings
using placeholders */
mrslt match(char *str, char *pattern)
{
    int first_ph = 0;
    int last_ph = 0;
    list pseg;
    /*placeholder corresponding substrings*/
    list substr;
    int i;
    int nomatch = 0;
    if (pattern[0] == PH)
        first_ph = 1;
    if (pattern[strlen(pattern) - 1] == PH)
        last_ph = 1;
    pseg = split(pattern, PH);
    char **pmatch; // pointers to current match
    pmatch = malloc(sizeof(char *) * (pseg.n));
    pmatch[0] = strstr(str, pseg.str[0]);
    substr.n = pseg.n - 1 + first_ph + last_ph;
    /*check if all segments appear in order in str
    pointers (locations) of matched segments are stored
    to extract substrings later
    */
    if (pmatch[0] != NULL)
    {
        for (i = 0; i < pseg.n - 1; i++)
        {
            pmatch[i + 1] = strstr(pmatch[i], pseg.str[i + 1]);
            if (pmatch[i + 1] == NULL)
            {
                nomatch = 1;
            }
        }
    }
    else
    {
        nomatch = 1;
    }
    //check if the first and last segment is at first and last position
    if(!nomatch){
    if(!first_ph&&(pmatch[0]!=str))
    nomatch=1;
    if(!last_ph&&(pmatch[pseg.n-1]+strlen(pseg.str[pseg.n-1])!=str+strlen(str)))
    nomatch=1;
    }
    if (!nomatch&&substr.n)
    {
        int c = 0; // counter
        int i;     // loop var
        /*tmp vars for copy length to reduce
        length of strcpy statement*/
        int l;
        /*tmp var for copy starting point, which is matched string
        plus offset for the matched segment,
        used to reduce length of strcpy statement*/
        char *b;
        
        substr.str = malloc(substr.n * sizeof(char *));
        if (first_ph)
        {
            substr.str[0] = malloc(pmatch[0] - str + 1);
            /*strncpy does not append \0, which will cause
            problems if malloc returned a non-zero memory,
            alternatively use calloc
            */
            strncpy(substr.str[0], str, pmatch[0] - str);
            substr.str[c][pmatch[0] - str] = '\0';
            c = 1;
        }
        for (i = 0; i < pseg.n - 1; i++)
        {
            l = pmatch[i + 1] - pmatch[i] - strlen(pseg.str[i]);
            b = pmatch[i] + strlen(pseg.str[i]);
            substr.str[c] = malloc(l + 1);
            strncpy(substr.str[c], b, l);
            substr.str[c][l] = '\0';
            c++;
        }
        if (last_ph)
        {
            // pseg.n is counter, but array counts from 0
            l = strlen(pmatch[pseg.n - 1]) - strlen(pseg.str[pseg.n - 1]);
            b = pmatch[pseg.n - 1] + strlen(pseg.str[pseg.n - 1]);
            substr.str[c] = malloc(l + 1);
            strncpy(substr.str[c], b, l);
            substr.str[c][l] = '\0';
        }
    }
    for (i = 0; i < pseg.n - 1; i++)
    {
        free(pseg.str[i]);
    }
    free(pmatch);
    if(nomatch)substr.n=0;
    mrslt result;
    result.matched=!nomatch;
    result.substr=substr;
    return result;
}
/*simple match, only match pattern, disregarding cond and action
match pattern of each rule in rset against str
, return the index of matched, where rset has rnum rules,
and store the extracted substrings in input_var
if no match, expected to return 0*/
int simp_rmatch(char *str, ruleset rs,varlist *vl)
{
    int i, j;
    int current_match=0;
    int wgt_high = 0;
    mrslt mresult;
    for (i = 0; i < rs.n; i++)
    {
        if (strcmp(rs.r[i][ptrn],""))
        {
            mresult = match(str, rs.r[i][ptrn]);
            if (mresult.matched && rs.w[i] > wgt_high)
            {
                //var[0] is $0, not used
                for (j = 0; j < mresult.substr.n; j++)
                {
                    strcpy(vl->var[j+1],mresult.substr.str[j]);
                }
                current_match = i;
                wgt_high = rs.w[i];
            }
            lfree(mresult.substr);
        }
    }
    return current_match;
}
/*substitute $var in pattern str and return the result*/
char *subs_var(varlist vlist,char *str)
{
    int i;
    // not empty str /""
    char *strbuf1 = malloc(strlen(str) + 1);
    char *strbuf2 = NULL;
    strcpy(strbuf1, str);
    for(i=0;i<vlist.n;i++)
    {if(strlen(vlist.var[i]) && strlen(vlist.name[i])){
        char *namebuf = malloc(100);
        strcpy(namebuf, "$");
        strcat(namebuf, vlist.name[i]);
        if (strbuf1 != NULL)
        {
            /*due to cannot predict the length after subs,
            memory is allocated during subs and returned*/
            strbuf2 = sub(namebuf, strbuf1, vlist.var[i]);
            free(strbuf1);
            strbuf1 = NULL;
        }
        else
        {
            strbuf1=sub(namebuf,strbuf2,vlist.var[i]);
            free(strbuf2);
            strbuf2=NULL;
        }
    }}
    if(strbuf1!=NULL)return strbuf1;
    else return strbuf2;
}
/*a simpler function that returns value for given varname,
in varlist, and does not malloc*/
char *getvar(varlist *vlist,char* vname){
    //using vlist instead of *vlist will cause returning local var
    int i=0;
    while(strcmp(vlist->name[i],vname)&&(i<vlist->n)){
        i++;
    }
    if(i>=vlist->n)return NULL;
    else return vlist->var[i];
}
/*append var with the given name to 
the (current) end of var list and set it to value*/
void add_namedvar(varlist *vl,char* name,char* value){
    strcpy(vl->name[vl->n],name);
    strcpy(vl->var[vl->n],value);
    vl->n++;
}
/*expand variables in given strings*/
void setvar(varlist *vlist,char *vname,char *value){
    char *value_sub;
    value_sub=subs_var(*vlist,value);
    int i=0;
    while(strcmp(vlist->name[i],vname)){
        i++;
    }
    strcpy(vlist->var[i],value_sub);
    free(value_sub);
}
void readrule(char *path, ruleset *rs,varlist *vl)
{
    FILE *f = fopen(path, "r");
    char cl[1000];//current line
    int initbegin=0;
    int rulebegin=0;
    list segbuf;
    int i;
    // number of sections of rule str, for checking
    int n;
    // counter for rules read and parsed
    int c = 0;
    if (f != NULL)
    {
        int cnum;
        while (fgets(cl, 1000, f))
        {
            trimn(cl);
            if(cl[0]!='['){
            if(initbegin){
                char *eqloc;
                eqloc=strchr(cl,'=');
                if (eqloc&&(cl[0]=='$'))
                {
                    char* namebuf=malloc(100);
                    strncpy(namebuf,cl+1,eqloc-cl-1);
                    namebuf[eqloc-cl-1]='\0';
                    add_namedvar(vl,namebuf,eqloc+1);
                    free(namebuf);
                }
                else
                {
                    fprintf(stderr,"Invalid init: %s\n",cl);
                } 
            }
            if(rulebegin){
            segbuf = split(cl, RDELIM);
            if (segbuf.n >= 3&&segbuf.n<=5)
            {
                strcpy(rs->r[c][ptrn], segbuf.str[0]);
                strcpy(rs->r[c][resp], segbuf.str[1]);
                sscanf(segbuf.str[2],"%d",&rs->w[c]);
                if(segbuf.n==4)
                strcpy(rs->r[c][cond],segbuf.str[2]);
                if(segbuf.n==5)
                strcpy(rs->r[c][actn],segbuf.str[3]);
                c++;
            }
            else{
                char *emsg="Invalid rule with %d sections: %s\n";
                fprintf(stderr,emsg,n,cl);
            }
            lfree(segbuf);
            }}
            else{
            if(!strcmp(cl,"[init]")){
                initbegin=1;
            }
            else if(!strcmp(cl,"[rule]")){
                initbegin=0;
                rulebegin=1;
            }}
        }
        fclose(f);
        rs->n=c;
    }
    else
    {
        printf("failed to read %s\n", path);
        exit(1);
    }
}
/*set a variable in code*/
void initsetvar(char *p,char *name,char *def,varlist *vl){
    if(getvar(vl,name)){
        strcpy(p,getvar(vl,name));
    }
    else strcpy(p,def);
}
#ifndef FUNCTEST
int main()
{
    ruleset rset=init_rset(100,100);
    varlist vlist=init_var();
    char *rfpath="./rule.conf";
    readrule(rfpath,&rset,&vlist);
    //init funcs...

    char usrname[100];
    initsetvar(usrname,"USER",DEFUNAME,&vlist);
    char chrname[100];
    initsetvar(chrname,"CHARA",DEFCNAME,&vlist);
    char csuffix[100];
    initsetvar(csuffix,"SUF","",&vlist); 
    char quitmsg[100];
    initsetvar(quitmsg,"QUIT",DEFQUITMSG,&vlist); 
    char emptymsg[100];
    initsetvar(emptymsg,"EMPTY",DEFEMPTMSG,&vlist); 
    //end of init funcs

    int flagquit=0;
    int matched_rnum=0;
    char usermsg[1000];
    char *response;
    while (!flagquit)
    {
        printf("%s:",usrname);
        if(!fgets(usermsg,1000,stdin))flagquit=1;
        //preproc funcs...
        trimn(usermsg);
        lower(usermsg,0);
        if(!strcmp(usermsg,quitmsg)){
            flagquit=1;
        }
        //end of preproc
        matched_rnum=simp_rmatch(usermsg,rset,&vlist);
        //postproc

        if(strlen(usermsg)==0)response=emptymsg;
        //end postproc
        response=subs_var(vlist,rset.r[matched_rnum][resp]);
        printf("%s:%s%s\n",chrname,response,csuffix);
        free(response);
    }
    return 0;
}
#endif
