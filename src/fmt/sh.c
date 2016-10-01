8600 // Shell.
8601 
8602 #include "types.h"
8603 #include "user.h"
8604 #include "fcntl.h"
8605 
8606 // Parsed command representation
8607 #define EXEC  1
8608 #define REDIR 2
8609 #define PIPE  3
8610 #define LIST  4
8611 #define BACK  5
8612 
8613 #define MAXARGS 10
8614 
8615 struct cmd {
8616   int type;
8617 };
8618 
8619 struct execcmd {
8620   int type;
8621   char *argv[MAXARGS];
8622   char *eargv[MAXARGS];
8623 };
8624 
8625 struct redircmd {
8626   int type;
8627   struct cmd *cmd;
8628   char *file;
8629   char *efile;
8630   int mode;
8631   int fd;
8632 };
8633 
8634 struct pipecmd {
8635   int type;
8636   struct cmd *left;
8637   struct cmd *right;
8638 };
8639 
8640 struct listcmd {
8641   int type;
8642   struct cmd *left;
8643   struct cmd *right;
8644 };
8645 
8646 struct backcmd {
8647   int type;
8648   struct cmd *cmd;
8649 };
8650 int fork1(void);  // Fork but panics on failure.
8651 void panic(char*);
8652 struct cmd *parsecmd(char*);
8653 
8654 // Execute cmd.  Never returns.
8655 void
8656 runcmd(struct cmd *cmd)
8657 {
8658   int p[2];
8659   struct backcmd *bcmd;
8660   struct execcmd *ecmd;
8661   struct listcmd *lcmd;
8662   struct pipecmd *pcmd;
8663   struct redircmd *rcmd;
8664 
8665   if(cmd == 0)
8666     exit();
8667 
8668   switch(cmd->type){
8669   default:
8670     panic("runcmd");
8671 
8672   case EXEC:
8673     ecmd = (struct execcmd*)cmd;
8674     if(ecmd->argv[0] == 0)
8675       exit();
8676     exec(ecmd->argv[0], ecmd->argv);
8677     printf(2, "exec %s failed\n", ecmd->argv[0]);
8678     break;
8679 
8680   case REDIR:
8681     rcmd = (struct redircmd*)cmd;
8682     close(rcmd->fd);
8683     if(open(rcmd->file, rcmd->mode) < 0){
8684       printf(2, "open %s failed\n", rcmd->file);
8685       exit();
8686     }
8687     runcmd(rcmd->cmd);
8688     break;
8689 
8690   case LIST:
8691     lcmd = (struct listcmd*)cmd;
8692     if(fork1() == 0)
8693       runcmd(lcmd->left);
8694     wait();
8695     runcmd(lcmd->right);
8696     break;
8697 
8698 
8699 
8700   case PIPE:
8701     pcmd = (struct pipecmd*)cmd;
8702     if(pipe(p) < 0)
8703       panic("pipe");
8704     if(fork1() == 0){
8705       close(1);
8706       dup(p[1]);
8707       close(p[0]);
8708       close(p[1]);
8709       runcmd(pcmd->left);
8710     }
8711     if(fork1() == 0){
8712       close(0);
8713       dup(p[0]);
8714       close(p[0]);
8715       close(p[1]);
8716       runcmd(pcmd->right);
8717     }
8718     close(p[0]);
8719     close(p[1]);
8720     wait();
8721     wait();
8722     break;
8723 
8724   case BACK:
8725     bcmd = (struct backcmd*)cmd;
8726     if(fork1() == 0)
8727       runcmd(bcmd->cmd);
8728     break;
8729   }
8730   exit();
8731 }
8732 
8733 int
8734 getcmd(char *buf, int nbuf)
8735 {
8736   printf(2, "$ ");
8737   memset(buf, 0, nbuf);
8738   gets(buf, nbuf);
8739   if(buf[0] == 0) // EOF
8740     return -1;
8741   return 0;
8742 }
8743 
8744 
8745 
8746 
8747 
8748 
8749 
8750 int
8751 main(void)
8752 {
8753   static char buf[100];
8754   int fd;
8755 
8756   // Ensure that three file descriptors are open.
8757   while((fd = open("console", O_RDWR)) >= 0){
8758     if(fd >= 3){
8759       close(fd);
8760       break;
8761     }
8762   }
8763 
8764   // Read and run input commands.
8765   while(getcmd(buf, sizeof(buf)) >= 0){
8766     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
8767       // Chdir must be called by the parent, not the child.
8768       buf[strlen(buf)-1] = 0;  // chop \n
8769       if(chdir(buf+3) < 0)
8770         printf(2, "cannot cd %s\n", buf+3);
8771       continue;
8772     }
8773     if(fork1() == 0)
8774       runcmd(parsecmd(buf));
8775     wait();
8776   }
8777   exit();
8778 }
8779 
8780 void
8781 panic(char *s)
8782 {
8783   printf(2, "%s\n", s);
8784   exit();
8785 }
8786 
8787 int
8788 fork1(void)
8789 {
8790   int pid;
8791 
8792   pid = fork();
8793   if(pid == -1)
8794     panic("fork");
8795   return pid;
8796 }
8797 
8798 
8799 
8800 // Constructors
8801 
8802 struct cmd*
8803 execcmd(void)
8804 {
8805   struct execcmd *cmd;
8806 
8807   cmd = malloc(sizeof(*cmd));
8808   memset(cmd, 0, sizeof(*cmd));
8809   cmd->type = EXEC;
8810   return (struct cmd*)cmd;
8811 }
8812 
8813 struct cmd*
8814 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
8815 {
8816   struct redircmd *cmd;
8817 
8818   cmd = malloc(sizeof(*cmd));
8819   memset(cmd, 0, sizeof(*cmd));
8820   cmd->type = REDIR;
8821   cmd->cmd = subcmd;
8822   cmd->file = file;
8823   cmd->efile = efile;
8824   cmd->mode = mode;
8825   cmd->fd = fd;
8826   return (struct cmd*)cmd;
8827 }
8828 
8829 struct cmd*
8830 pipecmd(struct cmd *left, struct cmd *right)
8831 {
8832   struct pipecmd *cmd;
8833 
8834   cmd = malloc(sizeof(*cmd));
8835   memset(cmd, 0, sizeof(*cmd));
8836   cmd->type = PIPE;
8837   cmd->left = left;
8838   cmd->right = right;
8839   return (struct cmd*)cmd;
8840 }
8841 
8842 
8843 
8844 
8845 
8846 
8847 
8848 
8849 
8850 struct cmd*
8851 listcmd(struct cmd *left, struct cmd *right)
8852 {
8853   struct listcmd *cmd;
8854 
8855   cmd = malloc(sizeof(*cmd));
8856   memset(cmd, 0, sizeof(*cmd));
8857   cmd->type = LIST;
8858   cmd->left = left;
8859   cmd->right = right;
8860   return (struct cmd*)cmd;
8861 }
8862 
8863 struct cmd*
8864 backcmd(struct cmd *subcmd)
8865 {
8866   struct backcmd *cmd;
8867 
8868   cmd = malloc(sizeof(*cmd));
8869   memset(cmd, 0, sizeof(*cmd));
8870   cmd->type = BACK;
8871   cmd->cmd = subcmd;
8872   return (struct cmd*)cmd;
8873 }
8874 
8875 
8876 
8877 
8878 
8879 
8880 
8881 
8882 
8883 
8884 
8885 
8886 
8887 
8888 
8889 
8890 
8891 
8892 
8893 
8894 
8895 
8896 
8897 
8898 
8899 
8900 // Parsing
8901 
8902 char whitespace[] = " \t\r\n\v";
8903 char symbols[] = "<|>&;()";
8904 
8905 int
8906 gettoken(char **ps, char *es, char **q, char **eq)
8907 {
8908   char *s;
8909   int ret;
8910 
8911   s = *ps;
8912   while(s < es && strchr(whitespace, *s))
8913     s++;
8914   if(q)
8915     *q = s;
8916   ret = *s;
8917   switch(*s){
8918   case 0:
8919     break;
8920   case '|':
8921   case '(':
8922   case ')':
8923   case ';':
8924   case '&':
8925   case '<':
8926     s++;
8927     break;
8928   case '>':
8929     s++;
8930     if(*s == '>'){
8931       ret = '+';
8932       s++;
8933     }
8934     break;
8935   default:
8936     ret = 'a';
8937     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
8938       s++;
8939     break;
8940   }
8941   if(eq)
8942     *eq = s;
8943 
8944   while(s < es && strchr(whitespace, *s))
8945     s++;
8946   *ps = s;
8947   return ret;
8948 }
8949 
8950 int
8951 peek(char **ps, char *es, char *toks)
8952 {
8953   char *s;
8954 
8955   s = *ps;
8956   while(s < es && strchr(whitespace, *s))
8957     s++;
8958   *ps = s;
8959   return *s && strchr(toks, *s);
8960 }
8961 
8962 struct cmd *parseline(char**, char*);
8963 struct cmd *parsepipe(char**, char*);
8964 struct cmd *parseexec(char**, char*);
8965 struct cmd *nulterminate(struct cmd*);
8966 
8967 struct cmd*
8968 parsecmd(char *s)
8969 {
8970   char *es;
8971   struct cmd *cmd;
8972 
8973   es = s + strlen(s);
8974   cmd = parseline(&s, es);
8975   peek(&s, es, "");
8976   if(s != es){
8977     printf(2, "leftovers: %s\n", s);
8978     panic("syntax");
8979   }
8980   nulterminate(cmd);
8981   return cmd;
8982 }
8983 
8984 struct cmd*
8985 parseline(char **ps, char *es)
8986 {
8987   struct cmd *cmd;
8988 
8989   cmd = parsepipe(ps, es);
8990   while(peek(ps, es, "&")){
8991     gettoken(ps, es, 0, 0);
8992     cmd = backcmd(cmd);
8993   }
8994   if(peek(ps, es, ";")){
8995     gettoken(ps, es, 0, 0);
8996     cmd = listcmd(cmd, parseline(ps, es));
8997   }
8998   return cmd;
8999 }
9000 struct cmd*
9001 parsepipe(char **ps, char *es)
9002 {
9003   struct cmd *cmd;
9004 
9005   cmd = parseexec(ps, es);
9006   if(peek(ps, es, "|")){
9007     gettoken(ps, es, 0, 0);
9008     cmd = pipecmd(cmd, parsepipe(ps, es));
9009   }
9010   return cmd;
9011 }
9012 
9013 struct cmd*
9014 parseredirs(struct cmd *cmd, char **ps, char *es)
9015 {
9016   int tok;
9017   char *q, *eq;
9018 
9019   while(peek(ps, es, "<>")){
9020     tok = gettoken(ps, es, 0, 0);
9021     if(gettoken(ps, es, &q, &eq) != 'a')
9022       panic("missing file for redirection");
9023     switch(tok){
9024     case '<':
9025       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
9026       break;
9027     case '>':
9028       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9029       break;
9030     case '+':  // >>
9031       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9032       break;
9033     }
9034   }
9035   return cmd;
9036 }
9037 
9038 
9039 
9040 
9041 
9042 
9043 
9044 
9045 
9046 
9047 
9048 
9049 
9050 struct cmd*
9051 parseblock(char **ps, char *es)
9052 {
9053   struct cmd *cmd;
9054 
9055   if(!peek(ps, es, "("))
9056     panic("parseblock");
9057   gettoken(ps, es, 0, 0);
9058   cmd = parseline(ps, es);
9059   if(!peek(ps, es, ")"))
9060     panic("syntax - missing )");
9061   gettoken(ps, es, 0, 0);
9062   cmd = parseredirs(cmd, ps, es);
9063   return cmd;
9064 }
9065 
9066 struct cmd*
9067 parseexec(char **ps, char *es)
9068 {
9069   char *q, *eq;
9070   int tok, argc;
9071   struct execcmd *cmd;
9072   struct cmd *ret;
9073 
9074   if(peek(ps, es, "("))
9075     return parseblock(ps, es);
9076 
9077   ret = execcmd();
9078   cmd = (struct execcmd*)ret;
9079 
9080   argc = 0;
9081   ret = parseredirs(ret, ps, es);
9082   while(!peek(ps, es, "|)&;")){
9083     if((tok=gettoken(ps, es, &q, &eq)) == 0)
9084       break;
9085     if(tok != 'a')
9086       panic("syntax");
9087     cmd->argv[argc] = q;
9088     cmd->eargv[argc] = eq;
9089     argc++;
9090     if(argc >= MAXARGS)
9091       panic("too many args");
9092     ret = parseredirs(ret, ps, es);
9093   }
9094   cmd->argv[argc] = 0;
9095   cmd->eargv[argc] = 0;
9096   return ret;
9097 }
9098 
9099 
9100 // NUL-terminate all the counted strings.
9101 struct cmd*
9102 nulterminate(struct cmd *cmd)
9103 {
9104   int i;
9105   struct backcmd *bcmd;
9106   struct execcmd *ecmd;
9107   struct listcmd *lcmd;
9108   struct pipecmd *pcmd;
9109   struct redircmd *rcmd;
9110 
9111   if(cmd == 0)
9112     return 0;
9113 
9114   switch(cmd->type){
9115   case EXEC:
9116     ecmd = (struct execcmd*)cmd;
9117     for(i=0; ecmd->argv[i]; i++)
9118       *ecmd->eargv[i] = 0;
9119     break;
9120 
9121   case REDIR:
9122     rcmd = (struct redircmd*)cmd;
9123     nulterminate(rcmd->cmd);
9124     *rcmd->efile = 0;
9125     break;
9126 
9127   case PIPE:
9128     pcmd = (struct pipecmd*)cmd;
9129     nulterminate(pcmd->left);
9130     nulterminate(pcmd->right);
9131     break;
9132 
9133   case LIST:
9134     lcmd = (struct listcmd*)cmd;
9135     nulterminate(lcmd->left);
9136     nulterminate(lcmd->right);
9137     break;
9138 
9139   case BACK:
9140     bcmd = (struct backcmd*)cmd;
9141     nulterminate(bcmd->cmd);
9142     break;
9143   }
9144   return cmd;
9145 }
9146 
9147 
9148 
9149 
