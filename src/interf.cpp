/***************************************************
****************************************************
LSD 6.4 - January 2015
written by Marco Valente
Universita' dell'Aquila

Copyright Marco Valente
Lsd is distributed according to the GNU Public License

Comments and bug reports to marco.valente@univaq.it
****************************************************
****************************************************/



/*
USED CASE 73
*/

/****************************************************
INTERF.CPP Manage the main interfaces, that the browser and all the menus.
It is re-build any time the window changes. There are some 20 actions that are
commanded from the browser window, implemented as a switch in operate.

The functions contained in this file are:

-object *create( object *root)
The main cycle for the Browser, from which it exits only to run a simulation
or to quit the program. The cycle is just once call to browsw followed by
a call to operate.

- int browse( object *r, int *choice);
build the browser window and waits for an action (on the form of
values for choice or choice_g different from 0)

- object *operate( int *choice, object *r);
takes the value of choice and operate the relative command on the
object r. See the switch for the complete list of the available commands

- void clean_debug(object *n);
remove all the flags to debug from any variable in the model

- void clean_save(object *n);
remove all the flags to save from any variable in the model

- void show_save(object *n)
shows all variables to be saved in the result files

- void clean_plot(object *n);
remove all the flags to plot from any variable in the model

- void wipe_out(object *d);
Eliminate all the Object like d from the model. Cancel also the their descendants


Functions used here from other files are:

- void plog(char *m);
LSDMAIN.CPP print  message string m in the Log screen.

- void analysis(int *choice);
ANALYSIS.CPP analysis of result files

- void show_eq(char *lab, int *choice);
SHOW_EQ.CPP shows one equation for variable lab

- object *skip_next_obj(object *t, int *i);
UTIL.CPP. Counts how many types of objects equal to t are in this
group. count returns such value, and the whole function returns the next object
after the last of the series.

- int my_strcmp(char *a, char *b);
UTIL.CPP It is a normal strcmp, but it catches the possibility of both strings being
NULL

- void cmd(Tcl_Interp *inter, char *cc);
UTIL.CPP Standard routine to send the message string cc to the interp
Basically it makes a simple Tcl_Eval, but controls also that the interpreter
did not issue an error message.

- object *go_brother(object *cur);
UTIL.CPP returns: c->next, if it is of the same type of c (brother).
Returns NULL otherwise. It is safe to use even when c or c->next are NULL.

- void show_graph( object *t);
DRAW.CPP shows the grsphical representation of the model

- void set_obj_number(object *r, int *choice);
EDIT.CPP allows to edit the number of instances in the model

- void edit_data(object *root, int *choice, char *obj_name);
EDIT_DAT.CPP allows to edit the initial values

- FILE *search_str(char *name, char *str);
UTIL.CPP given a string name, returns the file corresponding to name, and the current
position of the file is just after str.

- int deb(object *r, object *c, char *lab, double *res);
Use the debugger interface to browse through the model

- int cd(char *patch)
The change directory function uses different headers for Windows and Unix
Therefore, it is defined in the mainwin/main_un.

- void myexit(int v);
Exit function, which is customized on the operative system.

****************************************************/



#include <sys/stat.h>
#include <tk.h>
#include "decl.h"

#ifndef ACCESSPERMS
#define ACCESSPERMS 0777 
#endif

#define MAX_SENS_POINTS 999		// default warning threshold for sensitivity analysis

object *operate( int *choice, object *r);
int browse( object *r, int *choice);
object *skip_next_obj(object *t, int *i);
object *skip_next_obj(object *t);
int my_strcmp(char *a, char *b);
void cmd(Tcl_Interp *inter, char const *cc);
//void cmd(Tcl_Interp *inter, const char cc[]) {cmd(inter, (char *)cc);};
object *go_brother(object *cur);
void show_graph( object *t);
void set_obj_number(object *r, int *choice);
void edit_data(object *root, int *choice, char *obj_name);
void clean_debug(object *n);
void clean_save(object *n);
void show_save(object *n);
void show_initial(object *n);
void show_observe(object *n);
void set_window_size(void);

void clean_plot(object *n);
FILE *search_str(char const *name, char const *str);
void plog(char const *msg);
void analysis(int *choice);
void show_eq(char *lab, int *choice);
void chg_obj_num(object **c, int value, int all, int pippo[], int *choice, int cfrom);
int deb(object *r, object *c, char const *lab, double *res);
void wipe_out(object *d);
int check_label(char *l, object *r);
int cd(char *path);
void set_blueprint(object *container, object *r);

void myexit(int i);
void scan_used_lab(char *lab, int *choice);
void scan_using_lab(char *lab, int *choice);
void report(int *choice, object *r);
void empty_cemetery(void);
void createmodelhelp(int *choice, object *r);
description *search_description(char *lab);
void change_descr_lab(char const *lab_old, char const *lab, char const *type, char const *text, char const *init);
void change_descr_text(char *lab);
void change_init_text(char *lab);
void empty_descr(void);
void show_description(char *lab);
void add_description(char const *lab, char const *type, char const *text);
void auto_document(int *choice, char const *lab, char const *which);
void create_logwindow(void);
void delete_bridge(object *d);
void control_tocompute(object *r, char *ch);
int compute_copyfrom(object *c, int *choice);
int reset_bridges(object *r);
char *choose_object(void);
void insert_lb_object(object *r);
void save_eqfile(FILE *f);
void load_description( FILE *f);
void autofill_descr(object *o);
void read_eq_filename(char *s);
void save_description(object *r, FILE *f);
void tex_report(object *r, FILE *f);
void tex_report_init(object *r, FILE *f);
void tex_report_observe(object *r, FILE *f);
void shift_var(int direction, char *vlab, object *r);
void shift_desc(int direction, char *dlab, object *r);
object *sensitivity_parallel(object *o, sense *s );
void sensitivity_sequential(long *findexSens, sense *s, double probSampl = 1.0);
void sensitivity_doe( long *findex, design *doe );
long num_sensitivity_points( sense *rsens );	// calculates the sensitivity space size
int num_sensitivity_variables( sense *rsens );	// calculates the number of variables to test
void empty_sensitivity(sense *cs);
void set_all(int *choice, object *original, char *lab, int lag);
void dataentry_sensitivity(int *choice, sense *s, int nval);
bool discard_change( bool checkSense = true );	// ask before discarding unsaved changes
bool discard_sense( void );						// ask before discarding unsaved sensitivity changes

extern object *root;
extern char *simul_name;
extern char *struct_file;
extern char *exec_file;		// name of executable file
extern char *exec_path;		// path of executable file
extern int add_to_tot;
extern bool dozip;			// compressed results file flag
extern int struct_loaded;
extern char *path;
extern char *equation_name;
extern char name_rep[400];
extern int debug_flag;
extern int stackinfo_flag;
extern int t;
extern int optimized;
extern int check_optimized;
extern int when_debug;

extern Tcl_Interp *inter;
extern int seed;
extern int sim_num;
extern int max_step;
extern char msg[];
char *res_g;
extern int choice_g;
extern int choice;
extern variable *cemetery;
int result_loaded;
extern int message_logged;
extern int actual_steps;
extern description *descr;
extern int no_error;
extern char *eq_file;
extern char lsd_eq_file[];
int ignore_eq_file=1;
extern int lattice_type;
extern int no_res;
extern object *blueprint;
extern sense *rsense;
extern long nodesSerial;	// network node serial number global counter

char lastObj[256]="";		// to save last shown object for quick reload (choice=38)
char *sens_file=NULL;		// current sensitivity analysis file
long findexSens=0;			// index to sequential sensitivity configuration filenames
int strWindowOn=1;			// control the presentation of the model structure window
bool justAddedVar=false;	// control the selection of last added variable
bool unsavedChange = false;	// control for unsaved changes in configuration
bool unsavedSense = false;	// control for unsaved changes in sensitivity data
 
/****************************************************
CREATE
****************************************************/
object *create( object *cr)
{

object *cur;

#ifdef DUAL_MONITOR
// procedure to save top window size
cmd( inter, "proc save_top_size { } { scan [ wm geometry . ] \"%dx%d%*s\" w h; variable wB $w; variable hB $h }" );
// procedure to restore top window size
cmd( inter, "proc restore_top_size { } { variable wB; variable hB; scan [ wm geometry . ] \"%dx%d%*s\" w h; if { $wB != $w || $hB != $h } { wm geometry . \"[ expr $wB ]x$hB\" } }" );
// set and save top window initial configuration (run only once)
cmd( inter, "if { [ info exists wB ] == 0 || [ info exists wB ] == 0 } { wm geometry . \"[ expr $widthB ]x$heightB\"; scan [ wm geometry . ] \"%dx%d%*s\" wB hB }" );
#endif



cmd(inter, "set listfocus 1");
cmd(inter, "set itemfocus 0");

cmd(inter, "set choice -1");
cmd(inter, "set c \"\"");
Tcl_LinkVar(inter, "strWindowOn", (char*)&strWindowOn, TCL_LINK_BOOLEAN);
//Tcl_LinkVar(inter, "choice", (char *) &choice, TCL_LINK_INT);


Tcl_LinkVar(inter, "choice_g", (char *)&choice_g, TCL_LINK_INT);
//cmd(inter, "wm focusmodel . active");
choice_g=choice=0;
cmd(inter, "if { [winfo exist .log]==1} {wm resizable .log 1 1} {set choice -1}");
cmd(inter, "wm resizable . 1 1");
cmd(inter, "set cur 0"); //Set yview for vars listbox

sprintf(msg, "set ignore_eq_file %d", ignore_eq_file);
cmd(inter, msg);

sprintf(msg, "set lattype %d", lattice_type);
cmd(inter, msg);

if(strlen(lastObj)>0)		// restore previous object in browser, if any
{
	for(cur=cr; cur->up!=NULL; cur=cur->up);
	cur=cur->search(lastObj);
	if(cur!=NULL)
		cr=cur;
}

while(choice!=1) //Main Cycle ********************************
{
if(choice==-1)
 {
 cmd(inter, "set choice [winfo exists .log]"); 
 if(choice==0)
   create_logwindow();
 choice=0;
 }
 

sprintf(msg, "wm title . \"%s - Lsd Browser\"",simul_name);
cmd(inter, msg);

cmd(inter, "focus -force .");
for(cur=cr; cur->up!=NULL; cur=cur->up);

if(cur->v==NULL && cur->b==NULL)
  struct_loaded=0;
else
 { struct_loaded=1;
     show_graph(cr);
 if(message_logged==1)
  {
  cmd(inter, "wm deiconify .log");
  message_logged=0;
  }    
 }    

cmd(inter, "bind . <KeyPress-Escape> {}");
cmd(inter, "bind . <KeyPress-Return> {}");
//Draw the browser onto the ROOT Object

cmd(inter, "bind . <Destroy> {set choice 35}");
cmd(inter, "bind .log <Destroy> {set choice 35}");
if(choice!=55)
  choice=browse(cr, &choice);

/*
if(choice!=35)
 cmd(inter, "wm resizable . 0 0");
*/
//Perform the task selected
//cmd(inter, "set ugo 2");
//cmd(inter, "set ugo [winfo exists .model_str]");
//cmd(inter, "if { [winfo exists .model_str] == 1} {wm withdraw .model_str} {}");
cr=operate( &choice, cr);

//Delete the graphical representation


}
Tcl_UnlinkVar(inter,"save_option");
//cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
//Tcl_UnlinkVar(inter, "choice");
Tcl_UnlinkVar(inter, "choice_g");

cmd(inter, "if { [winfo exists .model_str] == 1} {wm withdraw .model_str} {}");
cmd(inter, "wm deiconify .log; wm deiconify .; raise .; focus -force .; update");
return(cr);
}



/****************************************************
BROWSE
****************************************************/
int browse(object *r, int *choice)
{
char ch[10000];
variable *ap_v;
int count, heightB, widthB;
object *ap_o;
bridge *cb;

if(*choice!=7 && *choice!=50 && *choice!=55)
 *choice=0;
//cmd(inter, "focus -force .");
cmd(inter, "frame .l -relief groove -bd 2");
cmd(inter, "frame .l.v -relief groove -bd 2");
cmd(inter, "frame .l.s -relief groove -bd 2");
cmd(inter, "label .l.v.lab -text Variables");
cmd(inter, "label .l.s.lab -text Descendants");

cmd(inter, "frame .l.v.c");
cmd(inter, "scrollbar .l.v.c.v_scroll -command \".l.v.c.var_name yview\"");
cmd(inter, "listbox .l.v.c.var_name -yscroll \".l.v.c.v_scroll set\"");

cmd(inter, "bind .l.v.c.var_name <Return> {set res [.l.v.c.var_name curselection]; set cur $res; if {$res !=\"\"} {set res [ .l.v.c.var_name get $res]; set listfocus 1; set itemfocus $cur ; set choice 7} {}}");
cmd(inter, "bind .l.v.c.var_name <Right> {focus -force .l.s.son_name; .l.s.son_name selection set 0}");

if(r->v==NULL)
  cmd(inter, ".l.v.c.var_name insert end \"(no Variables)\"");
else
  {
  cmd(inter, "set app 0");
  for(ap_v=r->v; ap_v!=NULL; ap_v=ap_v->next)
	 {
	  if(ap_v->param==1)
		 sprintf(ch, ".l.v.c.var_name insert end \"%s (Param.)\"",ap_v->label);
  	  if(ap_v->param==0)
		   sprintf(ch, ".l.v.c.var_name insert end \"%s (Var. lags=%d)\"",ap_v->label, ap_v->num_lag);
  	  if(ap_v->param==2)
       sprintf(ch, " .l.v.c.var_name insert end \"%s (Func. lags=%d)\"",ap_v->label, ap_v->num_lag);
	  cmd(inter, ch);

	  if(ap_v->param==0)
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg blue");
	  if(ap_v->param==1)
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg black");
	  if(ap_v->param==2)
		 sprintf(ch, ".l.v.c.var_name itemconf $app -fg red");
	  cmd(inter, ch);
    cmd(inter, "incr app");

	  if(ap_v->next == NULL && justAddedVar)	// last variable & just added a new variable?
	  {
		  justAddedVar=false;
		  cmd(inter, ".l.v.c.var_name selection clear 0 end; .l.v.c.var_name selection set end; set res [.l.v.c.var_name curselection]; set cur $res; if {$res !=\"\"} {set res [ .l.v.c.var_name get $res]; set listfocus 1; set itemfocus $cur}");
	  }
	 }
  }
if(r->v!=NULL)
  {cmd(inter, "bind .l.v.c.var_name <Double-Button-1> {set res [selection get]; set choice 7; set listfocus 1; set itemfocus [.l.v.c.var_name cur]; }");
	cmd(inter, "bind .l.v.c.var_name <Button-3> {.l.v.c.var_name selection clear 0 end;.l.v.c.var_name selection set @%x,%y; set listfocus 1; set itemfocus [.l.v.c.var_name cur]; set vname [selection get]; set choice 29; set cur [.l.v.c.var_name cur]}");
	cmd(inter, "bind .l.v.c.var_name <Button-2> {.l.v.c.var_name selection clear 0 end;.l.v.c.var_name selection set @%x,%y; set listfocus 1; set itemfocus [.l.v.c.var_name cur]; set vname [selection get]; set choice 29; set cur [.l.v.c.var_name cur]}");
	cmd( inter, "bind .l.v.c.var_name <Control-Up> { if { ! [ catch { set vname [ selection get ] } ] } { set listfocus 1; set choice 58; set itemfocus [ .l.v.c.var_name curselection ]; incr itemfocus -1 } }" );
	cmd( inter, "bind .l.v.c.var_name <Control-Down> { if { ! [ catch { set vname [ selection get ] } ] } { set listfocus 1; set choice 59; set itemfocus [ .l.v.c.var_name curselection ]; incr itemfocus } }" );
  }
cmd(inter, ".l.v.c.var_name yview $cur");


cmd(inter, "pack .l.v.c.v_scroll -side right -fill y");
cmd(inter, "listbox .l.s.son_name");
if(r->b==NULL)
  cmd(inter, ".l.s.son_name insert end \"(no Objects)\"");
else
 for(cb=r->b; cb!=NULL; cb=cb->next )
  {
	 strcpy(ch, ".l.s.son_name insert end ");
	 strcat(ch, cb->blabel);
   cmd(inter, ch);
	}
if(r->b!=NULL)
  cmd(inter, "bind .l.s.son_name <Double-Button-1> {set res [selection get]; set choice 4}");
cmd(inter, "bind .l.s.son_name <Return> {set res [.l.s.son_name curselection]; if {$res !=\"\"} {set res [ .l.s.son_name get $res]; set choice 4} {}}");
cmd( inter, "bind .l.s.son_name <Control-Up> { if { ! [ catch { set vname [ selection get ] } ] } { set choice 60; set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; incr itemfocus -1; .log.text.text.internal insert end \"$cur\"} }" );
cmd( inter, "bind .l.s.son_name <Control-Down> { if { ! [ catch { set vname [ selection get ] } ] } { set choice 61; set listfocus 2; set itemfocus [ .l.s.son_name curselection ]; incr itemfocus; .log.text.text.internal insert end \"$cur\"} }" );

cmd(inter, "bind .l.s.son_name <Left> {focus -force .l.v.c.var_name; set listfocus 1; set itemfocus 0; .l.v.c.var_name selection set 0; .l.v.c.var_name activate 0}");

//cmd(inter, "bind .l.s.son_name <Down> {.l.s.son_name selection clear 0 end; .l.s.son_name selection set active}");

cmd(inter, "frame .l.up_name");
cmd(inter, "label .l.up_name.d -text \"Parent Object: \"");
if( r->up==NULL)
  cmd(inter, "label .l.up_name.n -text \"(No Parent)\"");
else
 { strcpy(ch,"button .l.up_name.n -text \"");
	strcat(ch, (r->up)->label);
	strcat(ch, "\" -command {set itemfocus 0; set choice 5; } -foreground red -relief flat");
	cmd(inter, ch);

 }

cmd(inter, "bind . <KeyPress-u> {catch {.l.up_name.n invoke}   }");

cmd(inter, "pack .l.up_name.d .l.up_name.n -side left");

cmd(inter, "frame .l.tit -relief raised -bd 2");
//cmd(inter, "label .l.tit.lab -text \"Object Label: \" -font {System 12 bold}");
cmd(inter, "label .l.tit.lab -text \"Object: \"");
strcpy(ch, "button .l.tit.but -text ");
strcat(ch, r->label);
if(r->up!=NULL) 
 strcat(ch, " -command {set choice 6} -foreground red -relief flat");
else
 strcat(ch, " -command {} -foreground red -relief flat");
cmd(inter, ch);
 
cmd(inter, "pack .l.tit.lab .l.tit.but -side left");


cmd(inter, "menu .m -tearoff 0");

cmd(inter, "set w .m.file");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label File -menu $w -underline 0");
cmd(inter, "$w add command -label Load -command {set choice 17} -underline 0 -accelerator Ctrl+L");
cmd(inter, "$w add command -label \"Re-Load\" -command {set choice 38} -underline 0 -accelerator Ctrl+W");
cmd(inter, "$w add command -label Save -command {set choice 18} -underline 0 -accelerator Ctrl+S");
cmd(inter, "$w add command -label \"Save As\" -command {set choice 73} -underline 5");
cmd(inter, "$w add command -label Empty -command {set choice 20} -underline 0 -accelerator Ctrl+E");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Load Sensitivity Data\" -command {set choice 64} -underline 17");
cmd(inter, "$w add command -label \"Save Sensitivity Data\" -command {set choice 65} -underline 1");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label Quit -command {set choice 11} -underline 0 -accelerator Ctrl+Q");

cmd(inter, "set w .m.model");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Model -menu $w -underline 0");
cmd(inter, "$w add command -label \"Add a Variable\" -command {set param 0; set choice 2} -underline 6 -accelerator Ctrl+V");
cmd(inter, "$w add command -label \"Add a Parameter\" -command {set param 1; set choice 2} -underline 6 -accelerator Ctrl+P");
cmd(inter, "$w add command -label \"Add a Function\" -command {set param 2; set choice 2} -underline 8 -accelerator Ctrl+N");
cmd(inter, "$w add command -label \"Add a Descending Object\" -command {set choice 3} -underline 6 -accelerator Ctrl+D");
cmd(inter, "$w add command -label \"Insert New Parent\" -command {set choice 32} -underline 9");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Change Object\" -command {set choice 6} -underline 0");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Set Equation File Name\" -command {set choice 28} -underline 2 -accelerator Ctrl+U");
cmd(inter, "$w add checkbutton -label \"Ignore Equation File Controls \" -variable ignore_eq_file -command {set choice 54} -underline 0");
cmd(inter, "$w add command -label \"Upload Equation File \" -command {set choice 51} -underline 0");
cmd(inter, "$w add command -label \"Offload Equation File \" -command {set choice 52} -underline 0");
cmd(inter, "$w add command -label \"Compare Equation Files \" -command {set choice 53} -underline 2");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Generate Auto Descriptions \" -command {set choice 43} -underline 7");
cmd(inter, "$w add command -label \"Create Model Report \" -command {set choice 36} -underline 7 -accelerator Ctrl+C");
cmd(inter, "$w add command -label \"Generate LaTex report \" -command {set choice 57} -underline 9");

cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Find Element in Model\" -command {set choice 50} -underline 0 -accelerator Ctrl+F");
cmd(inter, "$w add checkbutton -label \"Enable Structure Window\" -variable strWindowOn -command {set choice 70} -underline 7 -accelerator Ctrl+Tab");

cmd(inter, "set w .m.data");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Data -menu $w -underline 0");
cmd(inter, "$w add cascade -label \"Set Number of Objects\" -underline 0 -menu $w.setobj");
cmd(inter, "$w add command -label \"Initial Values\" -command {set choice 21} -underline 0 -accelerator Ctrl+I");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Full Sensitivity (online)\" -command {set choice 62} -underline 18");
cmd(inter, "$w add command -label \"Full Sensitivity (batch)\" -command {set choice 63} -underline 0");
cmd(inter, "$w add command -label \"MC Sampling Sensit. (batch)\" -command {set choice 71} -underline 0");
cmd(inter, "$w add command -label \"NOLH Sampl. Sensit. (batch)\" -command {set choice 72} -underline 0");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Show Sensitivity Data\" -command {set choice 66} -underline 17");
cmd(inter, "$w add command -label \"Remove Sensitivity Data\" -command {set choice 67} -underline 1");
cmd(inter, "$w add command -label \"Create/Run Parallel Batch\" -command {set choice 68} -underline 0");

cmd(inter, "$w add separator");

cmd(inter, "$w add command -label \"Analysis of Results\" -command {set choice 26} -underline 0 -accelerator Ctrl+A");
cmd(inter, "$w add command -label \"Save Results\" -command {set choice 37}  -underline 5 -accelerator Ctrl+Z");
cmd(inter, "$w add command -label \"Data Browse\" -command {set choice 34} -underline 5 -accelerator Ctrl+B");

cmd(inter, "set w .m.data.setobj");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, "$w add command -label \"All types of objects\" -command {set choice 19} -accelerator Ctrl+O -underline 0");
cmd(inter, "$w add command -label \"Only current type of object\" -command {set choice 33} -underline 0");

cmd(inter, "set w .m.run");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Run -menu $w -underline 0");
cmd(inter, "$w add command -label Run -command {set choice 1} -underline 0 -accelerator Ctrl+R");
cmd(inter, "$w add command -label \"Start 'No Window' Batch\" -command {set choice 69} -underline 0");
cmd(inter, "$w add command -label \"Simulation Settings\" -command {set choice 22} -underline 2 -accelerator Ctrl+M");
cmd(inter, "$w add checkbutton -label \"Lattice updating\" -variable lattype -command {set choice 56} -underline 2");

cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Remove Debug Flags\" -command {set choice 27} -underline 13 -accelerator Ctrl+F");
cmd(inter, "$w add command -label \"Remove Save Flags\" -command {set choice 30} -underline 15 -accelerator Ctrl+G");
cmd(inter, "$w add command -label \"Remove Plot Flags\" -command {set choice 31} -underline 7");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Show Elements to Save\" -command {set choice 39} -underline 1");
cmd(inter, "$w add command -label \"Show Elements to Observe\" -command {set choice 42} -underline 17");
cmd(inter, "$w add command -label \"Show Elements to Initialize\" -command {set choice 49} -underline 17");
cmd(inter, "$w add separator");
cmd(inter, "$w add command -label \"Remove Runtime Plots\" -command {set choice 40} -underline 8");

cmd(inter, "set w .m.help");
cmd(inter, "menu $w -tearoff 0");
cmd(inter, ".m add cascade -label Help -menu $w -underline 0");
cmd(inter, "$w add command -label \"Help on Browser\" -command {LsdHelp Browser.html} -underline 0");
cmd(inter, "$w add command -label \"Lsd Quick Help\" -command {LsdHelp QuickHelp.html} -underline 0");
cmd(inter, "$w add separator");
cmd(inter, "if {$tcl_platform(platform) == \"unix\"} {$w add command -label \"Set Browser\" -command { set choice 48} -underline 0} {}");
cmd(inter, "$w add command -label \"Model Report\" -command {set choice 44} -underline 0");


cmd(inter, "bind . <Control-l> {set choice 17}");
cmd(inter, "bind . <Control-s> {set choice 18}");
cmd(inter, "bind . <Control-e> {set choice 20}");
cmd(inter, "bind . <Control-q> {set choice 11}");
cmd(inter, "bind . <Control-v> {set param 0; set choice 2}");
cmd(inter, "bind . <Control-p> {set param 1; set choice 2}");
cmd(inter, "bind . <Control-n> {set param 2; set choice 2}");

cmd(inter, "bind . <Control-d> {set choice 3}");
cmd(inter, "bind . <Control-o> {set choice 19}");
cmd(inter, "bind . <Control-i> {set choice 21}");
cmd(inter, "bind . <Control-a> {set choice 26}");
cmd(inter, "bind . <Control-r> {set choice 1}");
cmd(inter, "bind . <Control-m> {set choice 22}");
cmd(inter, "bind . <Control-f> {set choice 50}");
cmd(inter, "bind . <Control-u> {set choice 28}");
cmd(inter, "bind . <Control-g> {set choice 30}");
cmd(inter, "bind . <Control-b> {set choice 34}");
cmd(inter, "bind . <Control-c> {set choice 36}");
cmd(inter, "bind . <Control-z> {set choice 37}");
cmd(inter, "bind . <Control-w> {set choice 38}");
cmd(inter, "bind . <Control-Tab> {set strWindowOn [expr ! $strWindowOn]; set choice 70}");


cmd(inter, "bind .log <Control-l> {set choice 17}");
cmd(inter, "bind .log <Control-s> {set choice 18}");
cmd(inter, "bind .log <Control-e> {set choice 20}");
cmd(inter, "bind .log <Control-q> {set choice 11}");
cmd(inter, "bind .log <Control-v> {set param 0; set choice 2}");
cmd(inter, "bind .log <Control-p> {set param 1; set choice 2}");
cmd(inter, "bind .log <Control-n> {set param 2; set choice 2}");
cmd(inter, "bind .log <Control-d> {set choice 3}");
cmd(inter, "bind .log <Control-o> {set choice 19}");
cmd(inter, "bind .log <Control-i> {set choice 21}");
cmd(inter, "bind .log <Control-a> {set choice 26}");
cmd(inter, "bind .log <Control-r> {set choice 1}");
cmd(inter, "bind .log <Control-m> {set choice 22}");
cmd(inter, "bind .log <Control-f> {set choice 50}");
cmd(inter, "bind .log <Control-u> {set choice 28}");
cmd(inter, "bind .log <Control-g> {set choice 30}");
cmd(inter, "bind .log <Control-b> {set choice 34}");
cmd(inter, "bind .log <Control-z> {set choice 37}");
cmd(inter, "bind .log <Control-w> {set choice 38}");
cmd(inter, "bind .log <Control-Tab> {set strWindowOn [expr ! $strWindowOn]; set choice 70}");

if(strWindowOn)
{
cmd(inter, "bind $c <Control-l> {set choice 17}");
cmd(inter, "bind $c <Control-s> {set choice 18}");
cmd(inter, "bind $c <Control-e> {set choice 20}");
cmd(inter, "bind $c <Control-q> {set choice 11}");
cmd(inter, "bind $c <Control-v> {set param 0; set choice 2}");
cmd(inter, "bind $c <Control-p> {set param 1; set choice 2}");
cmd(inter, "bind $c <Control-n> {set param 2; set choice 2}");
cmd(inter, "bind $c <Control-d> {set choice 3}");
cmd(inter, "bind $c <Control-o> {set choice 19}");
cmd(inter, "bind $c <Control-i> {set choice 21}");
cmd(inter, "bind $c <Control-a> {set choice 26}");
cmd(inter, "bind $c <Control-r> {set choice 1}");
cmd(inter, "bind $c <Control-m> {set choice 22}");
cmd(inter, "bind $c <Control-f> {set choice 50}");
cmd(inter, "bind $c <Control-u> {set choice 28}");
cmd(inter, "bind $c <Control-g> {set choice 30}");
cmd(inter, "bind $c <Control-b> {set choice 34}");
cmd(inter, "bind $c <Control-c> {set choice 36}");
cmd(inter, "bind $c <Control-z> {set choice 37}");
cmd(inter, "bind $c <Control-w> {set choice 38}");
cmd(inter, "bind $c <Control-Tab> {set strWindowOn [expr ! $strWindowOn]; set choice 70}");
}

cmd(inter, ". configure -menu .m");


cmd(inter, "pack .l.v.c.var_name -fill both -expand yes");
//cmd(inter, "pack .l.v.lab .l.v.c -fill both -expand yes");
//cmd(inter, "pack .l.s.lab .l.s.son_name -fill both -expand yes");
cmd(inter, "pack .l.v.lab -fill x");
cmd(inter, "pack .l.v.c -fill both -expand yes");
cmd(inter, "pack .l.s.lab -fill x");
cmd(inter, "pack .l.s.son_name -fill both -expand yes");

cmd(inter, "pack .l.up_name .l.tit");
cmd(inter, "pack .l.v .l.s -side left -fill both -expand yes");

cmd(inter, "pack .l -fill both -expand yes");

//cmd(inter, "focus -force .l.v.c.var_name");
//cmd(inter, "update");



*choice=0;

#ifndef DUAL_MONITOR
cmd(inter, "set choice [info exist widthB]");
if(*choice==1)
  {
   //cmd(inter, "set posiz [format \"%dx%d+%d+%d\" $widthB $heightB $posX $posY]");
   cmd(inter, "set choice $heightB");
   heightB=*choice;
   cmd(inter, "set choice $widthB");
   widthB=*choice;
   sprintf(msg, "wm geometry . \"%dx%d+$posX+$posY\"; update", widthB, heightB);
   cmd(inter, msg);
   cmd(inter, "set choice [winfo height .]");
   if(heightB!=*choice)
    {//curiously engouh, in Win7 it needs two repetitions to properly set the geometry.
     sprintf(msg, "wm geometry . \"%dx%d+$posX+$posY\"", widthB, heightB);
     cmd(inter, msg);
    }    
//   plog("\n1a: $posiz"); 
//   cmd(inter, "set posiz [format \"%dx%d\" $widthB $heightB]");
//   cmd(inter, "wm geometry . \"$posiz\"; update");
   
   cmd(inter, "update");
//   plog("\n1aa: [wm geometry .]; [winfo height .]");
   *choice=0;
  } 
else
  {
   cmd(inter, "scan [wm geom .] %dx%d+%d+%d widthB heightB a b"); 
   cmd(inter, "wm geometry . +$posX+$posY; update");
//   plog("\n1b: [wm geometry .]"); 
  } 
cmd(inter, "set posXLog [expr $posX + $widthB +40]");
cmd(inter, "wm geometry .log +$posXLog+$posY");	
#endif

//cmd(inter, ".l.v.c.var_name selection set $cur");
//cmd(inter, ".l.v.c.var_name activate $cur");

cmd(inter, "update");
cmd(inter, "if { [info exists ModElem]==1 } {set ModElem [lsort -dictionary $ModElem]} {}");
main_cycle:
//cmd(inter, "focus -force .");

cmd(inter, "if { $listfocus == 1} {focus -force .l.v.c.var_name; .l.v.c.var_name selection set $itemfocus; .l.v.c.var_name activate $itemfocus; .l.v.c.var_name see $itemfocus} {}");
cmd(inter, "if { $listfocus == 2} {focus -force .l.s.son_name; .l.s.son_name selection set $itemfocus; .l.s.son_name activate $itemfocus} {}");
//cmd(inter, ".log.text.text.internal insert end \"\\nListfocus $listfocus; itemfocus $itemfocus\"");
if(*choice==50)
 {cmd(inter, "raise .log .");
  *choice=0;
 } 

#ifdef DUAL_MONITOR
// just restore size, if needed
cmd( inter, "restore_top_size" );		// restore top window size, if changed
#endif

while(*choice==0 && choice_g==0)
 Tcl_DoOneEvent(0);
 
#ifndef DUAL_MONITOR
cmd(inter, "scan [wm geom .] %dx%d+%d+%d widthB heightB posX posY");
//plog("\n2: [wm geometry .]");
#else
cmd( inter, "save_top_size" );		// save top window configuration before processing
#endif
 
if( *choice == 17 || *choice == 20 || *choice == 38 )	// reset only when really necessary
 cmd(inter, "set cur 0"); //Set yview for vars listbox

if(choice_g!=0)
 {*choice=choice_g;
  res_g=(char *)Tcl_GetVar(inter, "res_g",0);
  cmd(inter, "focus -force .l.v.c.var_name");
  choice_g=0;
 }

if(actual_steps>0)
 {
  if(*choice==1 || *choice==2 || *choice==3 || *choice==32 || *choice==6 || *choice==7 || *choice==28 || *choice==36 || *choice==43 || *choice==21 || *choice==19 || *choice==33 || *choice==22 || *choice==27 || *choice==30 || *choice==31 || *choice==25 || *choice==64 || *choice==65 || *choice==68 || *choice==69 )
   {
     cmd(inter, "toplevel .warn");
	 cmd(inter, "wm transient .warn .");
	 #ifdef DUAL_MONITOR
	 cmd(inter, "wm geometry .warn +[expr [winfo x .] + 20]+[expr [winfo y .] + 20]");
	 #endif
     cmd(inter, "label .warn.l -text \"Simulation just run.\nThe configuration currently loaded is the last step of the previous run.\nThe requested operation makes no sense on the final data of a simulation.\nChoose one of the followig options.\"");
     cmd(inter, "pack .warn.l");
     cmd(inter, "set temp 38");
     cmd(inter, "radiobutton .warn.reload -variable temp -value 38 -text \"Reload the current initial configuration\" -justify left -relief groove -bd 2 -anchor w");
     cmd(inter, "radiobutton .warn.load -variable temp -value 17 -text \"Load a new initial configuration\" -justify left -relief groove -bd 2 -anchor w");     
     cmd(inter, "radiobutton .warn.ar -variable temp -value 26 -text \"Analyse the results\" -justify left -relief groove -bd 2 -anchor w");     

   cmd(inter, "pack .warn.reload .warn.load .warn.ar -anchor w -fill x ");
   cmd(inter, "frame .warn.b");
   cmd(inter, "button .warn.b.ok -text \" Ok \" -command {set choice 1}");
   cmd(inter, "button .warn.b.esc -text \" Cancel \" -command {set choice 2}");
   cmd(inter, "button .warn.b.help -text \" Help \" -command {LsdHelp QuickHelp.html#problem}");
   cmd(inter, "pack .warn.b.ok .warn.b.help .warn.b.esc -side left");
   cmd(inter, "pack .warn.b");
   
   *choice=0;
   cmd(inter, "focus -force .warn.b.ok");

   cmd(inter, "bind .warn <Return> {.warn.b.ok invoke}");
   cmd(inter, "bind .warn <Escape> {.warn.b.esc invoke}");   
   while(*choice==0 && choice_g==0)
     Tcl_DoOneEvent(0);
   cmd(inter, "destroy .warn");
   if(*choice==1)
     cmd(inter, "set choice $temp");
   else 
     { *choice=0;  
       goto main_cycle;
     }  

   }
  } 
//cmd(inter, "set posX [winfo x .]");

//cmd(inter, "set posY [winfo y .]");
 
if(*choice!=35)
{cmd(inter, "if {[winfo exists .]==1} {bind . <Destroy> {}} {}");
 cmd(inter, "if {[winfo exists $c]==1} {bind $c <Destroy> {}} {}");
 cmd(inter,"if {[winfo exists .list]==1} {destroy .list} {}");
}


return *choice;
}

/****************************************************
OPERATE
****************************************************/
object *operate( int *choice, object *r)
{
char *lab1,*lab2,lab[300],lab_old[300], ch[300];
int sl, done=0, num, i, j, param, save, plot, nature, numlag, k, lag, temp[4];
bool saveAs;
char observe, initial, cc;
bridge *cb;

object *n, *cur, *cur1;
variable *cur_v, *cv, *app;
FILE *f;
result *rf;					// pointer for results files (may be zipped or not)
double fake=0;

sense *cs;

description *cur_descr;
//filled up to 45
switch(*choice)
{
//Add a Variable to the current object
case 2:


cmd(inter, "destroy .m .l");
Tcl_LinkVar(inter, "copy_param", (char *) &param, TCL_LINK_INT);
cmd(inter, "set copy_param $param");

cmd(inter, "button .ok -text Ok -command {set done 1}");
cmd(inter, "bind .ok <Return> {.ok invoke}");
cmd(inter, "button .cancel -text Cancel -command {set done 2}");
if(param==0)
 cmd(inter, "button .help -text Help -command {LsdHelp menumodel.html#AddAVar}");
else
 cmd(inter, "button .help -text Help -command {LsdHelp menumodel.html#AddAPar}");

cmd(inter, "bind . <KeyPress-Escape> {set done 2}");

cmd(inter, "set w .d");
cmd(inter, "frame .d");
cmd(inter, "frame $w.f");
cmd(inter, "label $w.f.lab -text \"Description\"");
cmd(inter, "pack $w.f.lab");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.text -expand yes -fill both");
cmd(inter, "pack $w.f");


Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
if(param==0)
{

cmd(inter, "set num 0");
cmd(inter, "set lab \"\"");
sprintf(msg, "label .l -text \"Insert a new Variable in Object %s\"", r->label);
cmd(inter, msg);
cmd(inter, "frame .f");
cmd(inter, "label .f.lab_ent -text \"New Variable Name: \"");
cmd(inter, "label .f.lab_num -text \" Maximum lags used\"");
cmd(inter, "entry .f.ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "entry .f.ent_num -width 2 -relief sunken -textvariable num");



cmd(inter, "bind .f.ent_var <KeyPress-Return> {focus -force .f.ent_num; .f.ent_num selection range 0 end}");
cmd(inter, "bind .f.ent_num <KeyPress-Return> {focus -force .ok}");
cmd(inter, "pack .f.lab_ent .f.ent_var .f.lab_num .f.ent_num -side left");
cmd(inter, "pack .l .f .d ");
cmd(inter, "pack .ok .help .cancel -side left -expand yes");
cmd(inter, "focus -force .f.ent_var");
}

if(param==2)
{

cmd(inter, "set num 0");
cmd(inter, "set lab \"\"");
sprintf(msg, "label .l -text \"Insert a new Function in Object %s\"", r->label);
cmd(inter, msg);
cmd(inter, "frame .f");
cmd(inter, "label .f.lab_ent -text \"New Function Name: \"");
cmd(inter, "label .f.lab_num -text \" Maximum lags used\"");
cmd(inter, "entry .f.ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "entry .f.ent_num -width 2 -relief sunken -textvariable num");



cmd(inter, "bind .f.ent_var <KeyPress-Return> {focus -force .f.ent_num; .f.ent_num selection range 0 end}");
cmd(inter, "bind .f.ent_num <KeyPress-Return> {focus -force .ok}");
cmd(inter, "pack .f.lab_ent .f.ent_var .f.lab_num .f.ent_num -side left");
cmd(inter, "pack .l .f .d ");
cmd(inter, "pack .ok .help .cancel -side left -expand yes");
cmd(inter, "focus -force .f.ent_var");
}

if(param==1)
{ //insert a parameter
cmd(inter, "set lab \"\"");
sprintf(msg, "label .l -text \"Insert a new Parameter in Object %s\"", r->label);
cmd(inter, msg);
cmd(inter, "frame .f");
cmd(inter, "label .f.lab_ent -text \"New Parameter Name: \"");
cmd(inter, "entry .f.ent_var -width 20 -relief sunken -textvariable lab");


cmd(inter, "bind .f.ent_var <KeyPress-Return> {focus -force .ok}");
cmd(inter, "pack .f.lab_ent .f.ent_var -side left");
cmd(inter, "pack .l .f .d -anchor w");
cmd(inter, "pack .ok .help .cancel -side left -expand yes");
cmd(inter, "focus -force .f.ent_var");
}
//cmd(inter, "raise .");

cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");

set_window_size();
while(done==0)
 Tcl_DoOneEvent(0);
cmd(inter, "set text_description [.d.f.text get 1.0 end]");

cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");

 cmd(inter, "destroy .l .f .d .ok .cancel .help");
if(done==1)
 {
lab1=(char *)Tcl_GetVar(inter, "lab",0);
strcpy(lab, lab1);
for(i=0; lab[i]!=(char )NULL; i++)
 if(lab[i]==' ')
  {lab[i]=(char)NULL;
   break ;
  } 
sl=strlen(lab);
if(sl!=0)
 {
 for(cur=r; cur->up!=NULL; cur=cur->up);
 done=check_label(lab, cur);
 if(done==1)
   {cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"The new label already exists in the model.\"");
    return r;
   }

 if(done==0)
 {
 cmd(inter, "if { $text_description==\"\\n\"} {set text_description \"(no description available)\"} {}");
 lab1=(char *)Tcl_GetVar(inter, "text_description",0);
 if(param==1)
  add_description(lab, "Parameter", lab1);
 if(param==0)
  add_description(lab, "Variable", lab1);
 if(param==2)
  add_description(lab, "Function", lab1);

 for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
  { cur->add_empty_var(lab);
	cv=cur->search_var(NULL, lab);
	if(param==1)
	 num=0;
	cv->val=new double[num+1];
	cv->save=0;
	cv->param=param;
	cv->num_lag=num;
	cv->debug='n';
	if((param==0 || param==2) && num==0)
	  cv->data_loaded='+';
	else
	  cv->data_loaded='-';

	for(i=0; i<num+1; i++)
	 cv->val[i]=0;
 
	justAddedVar=true;		// flag variable just added (for acquiring focus)
  }
  unsavedChange = true;		// signal unsaved change
 }
 }

 }

cmd(inter, "unset lab done");

if(done!=2)
 {
  sprintf(msg, "lappend ModElem %s", lab);
  cmd(inter, msg);
 }  
Tcl_UnlinkVar(inter, "done");
Tcl_UnlinkVar(inter, "num");
Tcl_UnlinkVar(inter, "copy_param");


break;


//Add a Descendent type to the object and assigns the number of its instances.
case 3:
cmd(inter, "destroy .m .l");
cmd(inter, "button .ok -text Ok -command {set done 1}");
cmd(inter, "button .cancel -text Cancel -command {set done 2}");
cmd(inter, "button .help -text Help -command {LsdHelp menumodel.html#AddADesc}");
cmd(inter, "bind . <KeyPress-Escape> {set done 2}");


Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);

cmd(inter, "set lab \"\"");
sprintf(msg, "label .tit -text \"Add a new Object type descending from: %s\"",r->label);
cmd(inter, msg);
cmd(inter, "frame .f");
cmd(inter, "label .f.lab_ent -text \"New Object Name: \"");
cmd(inter, "entry .f.ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "pack .f.lab_ent .f.ent_var -side left");


cmd(inter, "bind .f.ent_var <KeyPress-Return> {set done 1}");

cmd(inter, "pack .tit .f .ok .help .cancel");
cmd(inter, "focus .f.ent_var");

cmd(inter, "set w .d");
cmd(inter, "frame .d");
cmd(inter, "frame $w.f");
cmd(inter, "label $w.f.lab -text \"Description\"");
cmd(inter, "pack $w.f.lab");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "bind $w.f.text <KeyPress-Return> {}");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.text -expand yes -fill both");
cmd(inter, "pack $w.f");
cmd(inter, "pack .d");

cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
//cmd(inter, "wm iconify .log");

set_window_size();
here_newobject:
while(done==0)
 Tcl_DoOneEvent(0);

cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
if(done==1)
{
 lab1=(char *)Tcl_GetVar(inter, "lab",0);
 strcpy(lab, lab1);
 if(strlen(lab)==0)
  {
   cmd(inter, "destroy .ok .d .cancel .f .help .tit");
   Tcl_UnlinkVar(inter, "done");
   cmd(inter, "unset lab done"); 
   *choice=0;
   return(r);
  }
 cmd(inter, "set done [string is graph \"$lab\"]");
 if(done==0)
  {
   cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"The label contains forbidden characters, like spaces.\\nInsert a new label.\"");
   cmd(inter, "focus .f.ent_var; .f.ent_var selection range 0 end");
   goto here_newobject;
  }
 for(cur=r; cur->up!=NULL; cur=cur->up);
 done=check_label(lab, cur); //check that the label does not exist already

 if(done==0)
  {r->add_obj(lab, 1, 1);
   cmd(inter, "set text_description [.d.f.text get 1.0 end]");  
   cmd(inter, "if { $text_description==\"\\n\" || $text_description==\"\"} {set text_description \"(no description available )\"} {}");
   lab1=(char *)Tcl_GetVar(inter, "text_description",0);
   add_description(lab, "Object", lab1);

   unsavedChange = true;		// signal unsaved change
  }
 else
  cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"The new label already exists in the model.\"");
 }
cmd(inter, "destroy .ok .d .cancel .f .help .tit");
Tcl_UnlinkVar(inter, "done");
cmd(inter, "unset lab done");

break;

//Insert a parent Object just above the current object
case 32:
cmd(inter, "destroy .m .l");
Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
if(r->up==NULL)
 {
  cmd(inter, "label .l -text \"Cannot insert a parent of Root.\\nThe new Object will contain all current descendants from Root.\"");
  cmd(inter, "button .ok -text \" Ok \" -command {set done 1}");
  cmd(inter, "pack .l .ok");
  cmd(inter, "bind . <KeyPress-Return> {set done 1}");
  set_window_size();
  while(done==0)
   Tcl_DoOneEvent(0);
  done=0;
  cmd(inter, "destroy .l .ok");
 }
cmd(inter, "button .ok -text Ok -command {set done 1}");
cmd(inter, "button .cancel -text Cancel -command {set done 2}");
cmd(inter, "button .help -text Help -command {LsdHelp menumodel.html#InsertAParent}");


if(r->up!=NULL)
  sprintf(msg, "label .lab_ent -text \"Insert a new Object parent of %s and descending from %s\"", r->label, r->up->label);
else
  sprintf(msg, "label .lab_ent -text \"Insert the new Object label\"");

cmd(inter, msg);
cmd(inter, "set lab \"\"");
cmd(inter, "entry .ent_var -width 20 -relief sunken -textvariable lab");
cmd(inter, "bind . <KeyPress-Return> {set done 1}");
cmd(inter, "bind . <KeyPress-Escape> {set done 2}");
cmd(inter, "pack .lab_ent .ent_var .ok .help .cancel");
cmd(inter, "focus .ent_var");
cmd(inter, "bind . <KeyPress-Escape> {set done 2}");
cmd(inter, "bind . <KeyPress-Return> {set done 1}");

cmd(inter, "set w .d");
cmd(inter, "frame .d");
cmd(inter, "frame $w.f");
cmd(inter, "label $w.f.lab -text \"Description\"");
cmd(inter, "pack $w.f.lab");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "bind $w.f.text <KeyPress-Return> {}");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.text -expand yes -fill both");
cmd(inter, "pack $w.f");
cmd(inter, "pack .d");

set_window_size();
while(done==0)
 Tcl_DoOneEvent(0);

if(done==1)
{
 lab1=(char *)Tcl_GetVar(inter, "lab",0);
 strcpy(lab, lab1);
 for(cur=r; cur->up!=NULL; cur=cur->up);
 done=check_label(lab1, cur); //check that the label does not exist already
 if(done==1)
   {cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"The new label already exists in the model.\"");
    cmd(inter, "destroy .d .ok .help .cancel .lab_ent .ent_var");
    Tcl_UnlinkVar(inter, "done");
    cmd(inter, "unset lab done");
    return r;
   }

 if(r->up==NULL)
  {
  cur=new object;
  cur->init(NULL, lab);
  cur->next=NULL;

  cur->up=r;
  cur->to_compute=1;
  cur->b=r->b;
  r->b=new bridge;
  r->b->next=NULL;
  r->b->blabel=new char[strlen(lab)+1];
  strcpy(r->b->blabel, lab);
  
  r->b->head=cur;
  cur->v=r->v;
  r->v=NULL;
  for(cur1=cur->b->head; cur1!=NULL; cur1=cur1->next)
    cur1->up=cur;
  }
 else
  {r->insert_parent_obj_one(lab);
   r=r->up;
  }

 }

 cmd(inter, "set text_description [.d.f.text get 1.0 end]");  
 cmd(inter, "if { $text_description==\"\\n\" || $text_description==\"\"} {set text_description \"(no description available )\"} {}");
 lab1=(char *)Tcl_GetVar(inter, "text_description",0);
 add_description(lab, "Object", lab1);

 unsavedChange = true;		// signal unsaved change

cmd(inter, "destroy .d .ok .cancel .help .lab_ent .ent_var");
Tcl_UnlinkVar(inter, "done");
cmd(inter, "unset lab done");

//done=reset_bridges(root);
*choice=0;

break;

//Move browser to show one of the descendant object
case 4:

cmd(inter, "destroy .m .l");

*choice=0;

lab1=(char *)Tcl_GetVar(inter, "res",0);


if(!strcmp(lab1, "(no Objects)") )
 return(r);
 
n=r->search(lab1);
if(n==NULL)
 {sprintf(ch, "\nDescendant %s not found",lab1);
  plog(ch);
  return r;
 }
//else
strcpy(lastObj,lab1);		// save last shown object for quick reload (choice=38)
return (n);



//Move browser to show the parent object
case 5:
cmd(inter, "destroy .m .l");
if(r->up==NULL)
 return r;
for(i=0, cb=r->up->b; cb->head!=r; cb=cb->next, i++);
sprintf(msg, "set listfocus 2; set itemfocus %d", i);
cmd(inter, msg); 
strcpy(lastObj,r->up->label);		// save last shown object for quick reload (choice=38)
return r->up;


//Edit current Object's name and give the option to disable the computation
case 6:
cmd(inter, "destroy .m .l");
sprintf(msg, "set lab %s", r->label);
cmd(inter, msg);
lab1=(char *)Tcl_GetVar(inter, "lab",0);

sscanf(lab1, "%s", lab_old);



cmd(inter, "frame .b");
cmd(inter, "button .b.ok -text Continue -command {set choice 1}");
cmd(inter, "button .b.del -text DELETE -command {set choice 3}");
cmd(inter, "button .b.cancel -text Cancel -command {set choice 2}");
cmd(inter, "button .b.help -text Help -command {LsdHelp menumodel.html#ChangeObjName}");
cmd(inter, "pack .b.ok .b.del .b.help .b.cancel -side left -fill x -expand yes");

cmd(inter, "frame .b1");
sprintf(msg, "set to_compute %d",r->to_compute);
cmd(inter, msg);
cmd(inter, "checkbutton .b1.com -text \"Compute: require the simulation to compute the Vars. in this Object.\" -variable to_compute -anchor w");
cmd(inter, "pack .b1.com -anchor w");

cur_descr=search_description(lab_old);
if(cur_descr==NULL)
  {
   add_description(lab_old, "Object", "(no description available)");
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", lab_old);
   plog(msg);
   cur_descr=search_description(lab_old);
  } 
  
cmd(inter, "set w .desc");
cmd(inter, "frame $w -bd 2 -relief groove");
cmd(inter, "frame $w.f");
cmd(inter, "label $w.f.int -text \"Description of Object: $lab\"");
cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
cmd(inter, "text $w.f.text -wrap word -width 60 -height 10 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
cmd(inter, "pack $w.f.yscroll -side right -fill y");
cmd(inter, "pack $w.f.int $w.f.text -anchor w -expand yes -fill both");
cmd(inter, "pack $w.f");
  for(i=0; cur_descr->text[i]!=(char)NULL; i++)
   {
   if(cur_descr->text[i]!='[' && cur_descr->text[i]!=']' && cur_descr->text[i]!='{' && cur_descr->text[i]!='}' && cur_descr->text[i]!='\"' )
     {
      sprintf(msg, ".desc.f.text insert end \"%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
    else
     {
      sprintf(msg, ".desc.f.text insert end \"\\%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }

   }
 cmd(inter, "$w.f.text delete \"end - 1 char\"");
cmd(inter, "pack $w.f -fill x -expand yes");


cmd(inter, "frame .h -bd 2 -relief groove");
cmd(inter, "label .h.ent_var -width 30 -relief sunken -fg red -text $lab");

cmd(inter, "bind .h.ent_var <KeyPress-Return> {set choice 1}");
cmd(inter, "bind . <KeyPress-Escape> {set choice 2}");

cmd(inter, "label .h.lab_ent -text \"Object\"");

cmd(inter, "button .h.prop -text \"Change Name\" -command {set choice 5}" );
cmd(inter, "bind .h <Double-1> {set choice 5}");
cmd(inter, "bind .h.lab_ent <Double-1> {set choice 5}");
cmd(inter, "bind .h.ent_var <Double-1> {set choice 5}");
cmd(inter, "bind .h.ent_var <KeyPress-c> {.b1.com invoke}");
cmd(inter, "pack .h.lab_ent .h.ent_var .h.prop");
cmd(inter, "pack .h .b1 .b .desc -fill x -expand yes");
cmd(inter, "focus -force .h.ent_var");
cmd(inter, "bind . <Control-d> {}");
cmd(inter, "bind . <Control-z> {}");
cmd(inter, "bind .h.ent_var <Control-d> {focus -force .desc.f.text}");
cmd(inter, "bind .desc.f.text <Control-z> {set choice 1}");
//cmd(inter, "wm resizable . 0 0 ");
cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
*choice=0;
set_window_size();
while(*choice==0)
 Tcl_DoOneEvent(0);

cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");

cmd(inter, "set text_description \"[.desc.f.text get 1.0 end]\"");
cmd(inter, "destroy .h .b1 .b .desc");
if(*choice==1|| *choice==5 || *choice==3)
{

change_descr_text(lab_old);

unsavedChange = true;		// signal unsaved change
  
if(*choice==5 || *choice==3)
{
if(*choice==3)
{
	 cmd(inter, "set answer [tk_messageBox -title \"Delete Object\" -icon warning -type okcancel -default ok -message \"Press 'Ok' to confirm deleting:\n\n$lab\"]");
	 cmd(inter, "switch -- $answer {ok {set choice 1} cancel {set choice 2}}");
	 if(*choice == 1)				// simulate a name change
		cmd(inter, "set lab \"\"");	// to empty string (delete)
}
else
{
cmd(inter, "label .l -text \"New Label for Object $lab\"");
cmd(inter, "entry .e -width 30 -textvariable lab");
cmd(inter, "pack .l .e -anchor w");
cmd(inter, "frame .b");
cmd(inter, "button .b.ok -text Ok -command {set choice 1}");
cmd(inter, "button .b.esc -text Cancel -command {set choice 2}");
cmd(inter, "pack .b.ok .b.esc -side left");
cmd(inter, "pack .b");
cmd(inter, "focus -force .e");
*choice=0;

set_window_size();
while(*choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "destroy .l .e .b");
}
 if(*choice==2)
  {
   *choice=0;
   return r;
  }

lab1=(char *)Tcl_GetVar(inter, "lab",0);
if(strlen(lab1)!=0)
{

 strcpy(lab, lab1);
 if(strcmp(lab, r->label) )
  {
   for(cur=r; cur->up!=NULL; cur=cur->up);
   done=check_label(lab, cur); //check that the label does not exist already
   if(done==1)
     cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"The new label already exists in the model.\"");

  }
 else
  done=0;
 if(strcmp(lab, r->label) && done==0)
  {
   change_descr_lab(r->label, lab, "", "", "");
   r->chg_lab(lab);
   *choice=0;
   return r;
  }
}
else //Delete the Object !
 {for(cur=r->up; cur->up!=NULL; cur=cur->up);
  cur=cur->search(r->label);
  r=r->up;
  for(cv=cur->v; cv!=NULL; cv=cv->next)
    change_descr_lab(cv->label,"" , "", "", "");
  wipe_out(cur);
   *choice=0;
   return r;

 }
}//end of *choice==5
cmd(inter, "set choice $to_compute");

if(*choice!=r->to_compute)
{cur=blueprint->search(r->label);
 cur->to_compute=*choice;
 for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
   cur->to_compute=*choice;
}   
//control for elements to save in objects to be not computed
if(*choice==0)
{
control_tocompute(r, r->label);
}
}//end of *choice==1 || *choice==5

*choice=0;
return r;



//Edit variable name and set debug/saving/plot flags
case 7:
int savei;

cmd(inter, "destroy .m .l");
cmd(inter, "update");

lab1=(char *)Tcl_GetVar(inter, "res",0);

sscanf(lab1, "%s", lab_old);

if(!strcmp(lab1, "(no Variables)"))
 {
  *choice=0;
  return r;
 }
cv=r->search_var(NULL, lab_old);
Tcl_LinkVar(inter, "debug", (char *) &num, TCL_LINK_INT);
Tcl_LinkVar(inter, "save", (char *) &save, TCL_LINK_INT);
Tcl_LinkVar(inter, "savei", (char *) &savei, TCL_LINK_INT);

Tcl_LinkVar(inter, "plot", (char *) &plot, TCL_LINK_INT);

save=cv->save;
num=cv->debug=='d'?1:0;
plot=cv->plot;
savei=cv->savei;
 

cmd(inter, "frame .b");
cmd(inter, "button .b.ok -text Continue -command {set done 1}");
cmd(inter, "button .b.cancel -text Cancel -command {set done 2}");
cmd(inter, "button .b.help -text Help -command {LsdHelp Browser.html#variables}");
cmd(inter, "pack .b.ok .b.help .b.cancel -side left -fill x -expand yes");

cmd(inter, "frame .b1");
cmd(inter, "checkbutton .b1.deb -text \"Debug: allow interruption after this equation\" -variable debug -anchor w");
cmd(inter, "checkbutton .b1.sav -text \"Save: save these series for later analysis\" -variable save -anchor w");
cmd(inter, "checkbutton .b1.savi -text \"Save in a separate file each series\" -variable savei -anchor w");

cmd(inter, "checkbutton .b1.plt -text \"Run Time Plot: observe these series during the simulation\" -variable plot -anchor w");
cmd(inter, msg);
if(cv->param==1)
 cmd(inter, "pack .b1.sav .b1.savi .b1.plt -anchor w");
if(cv->param==0)
 cmd(inter, "pack .b1.deb .b1.sav .b1.savi .b1.plt -anchor w");
if(cv->param==2)
 cmd(inter, "pack .b1.deb .b1.sav .b1.savi -anchor w");

cmd(inter, "bind .b1 <KeyPress-d> {.b1.deb invoke}");
cmd(inter, "bind .b1 <KeyPress-s> {.b1.sav invoke}");
cmd(inter, "bind .b1 <KeyPress-p> {.b1.plt invoke}");
cmd(inter, "bind .b1 <KeyPress-i> {.desc.opt.ini invoke}");
cmd(inter, "bind .b1 <KeyPress-o> {.desc.opt.obs invoke}");

sprintf(ch, "set vname %s", lab_old);
cmd(inter, ch);

 cur_descr=search_description(lab_old);
 if(cur_descr==NULL)
  {if(cv->param==0)
     add_description(lab_old, "Variable", "(no description available)");
   if(cv->param==1)
     add_description(lab_old, "Parameter", "(no description available)");  
   if(cv->param==2)
     add_description(lab_old, "Function", "(no description available)");  
   sprintf(msg, "Warning! description for '%s' not found. New one created.\\n", lab_old);
   plog(msg);
   cur_descr=search_description(lab_old);
  } 

 cmd(inter, "set w .desc");
 cmd(inter, "frame $w -bd 2 -relief groove");
 cmd(inter, "frame $w.f");
 if(!strcmp(cur_descr->type,"Parameter") )
   cmd(inter, "label $w.f.int -text \"Description of Parameter $vname\"");
 else
   cmd(inter, "label $w.f.int -text \"Description of Variable $vname\"");

  cmd(inter, "scrollbar $w.f.yscroll -command \"$w.f.text yview\"");
 cmd(inter, "text $w.f.text -wrap word -width 60 -height 10 -relief sunken -yscrollcommand \"$w.f.yscroll set\"");
 cmd(inter, "pack $w.f.yscroll -side right -fill y");
 cmd(inter, "pack $w.f.int $w.f.text -anchor w -expand yes -fill both");
 cmd(inter, "pack $w.f");
  for(i=0; cur_descr->text[i]!=(char)NULL; i++)
   {
   if(cur_descr->text[i]!='[' && cur_descr->text[i]!=']' && cur_descr->text[i]!='{' && cur_descr->text[i]!='}' && cur_descr->text[i]!='\"' )
     {
      sprintf(msg, ".desc.f.text insert end \"%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
    else
     {
      sprintf(msg, ".desc.f.text insert end \"\\%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }

   }
 cmd(inter, "frame $w.opt");
 sprintf(msg, "set observe %d", cur_descr->observe=='y'?1:0);
 cmd(inter, msg);
 sprintf(msg, "set initial %d", cur_descr->initial=='y'?1:0);
 cmd(inter, msg);
 
 cmd(inter, "label $w.opt.l -text \"In model documentation set the element to be: \"");
 cmd(inter, "pack $w.opt.l -side left");
 cmd(inter, "checkbutton $w.opt.obs -text \"Observe\" -variable observe -anchor w");
 cmd(inter, "checkbutton $w.opt.ini -text \"Initialize\" -variable initial -anchor w"); 
 if(cv->param==1 || cv->num_lag>0)
  cmd(inter, "pack $w.opt.obs $w.opt.ini -side left -anchor w");
 else
  cmd(inter, "pack $w.opt.obs -anchor w");
 cmd(inter, "$w.f.text delete \"end - 1 char\"");
 cmd(inter, "pack $w.opt -expand yes -fill x");
 cmd(inter, "frame $w.b");
// cmd(inter, "button $w.b.save -text \"Save Description\" -command {set text_description [.desc.f.text get 1.0 end];set done 6}");
 cmd(inter, "button $w.b.eq -text \"See code\" -command {set done 3}");
 cmd(inter, "button $w.b.auto_doc -text \"Auto Docum.\" -command {set done 9}");
 cmd(inter, "button $w.b.us -text \"Eq. using this element\" -command {set done 4}");
 cmd(inter, "button $w.b.using -text \"Elements used\" -command {set done 7}");
 cmd(inter, "button $w.b.del -text \"DELETE element\" -command {set done 10}");
 if(!strcmp(cur_descr->type, "Parameter"))
   cmd(inter, "pack $w.b.auto_doc $w.b.us $w.b.del -side left -expand yes -fill both");
 else
   cmd(inter, "pack $w.b.eq $w.b.auto_doc $w.b.us $w.b.using $w.b.del -side left -expand yes -fill both");
 cmd(inter, "pack $w.f $w.b -fill x -expand yes");
 if(cv->param==1 || cv->num_lag>0)
  {
   cmd(inter, "frame $w.i");
   cmd(inter, "label $w.i.int -text \"Comments on the initial values of '$vname'\"");
   cmd(inter, "scrollbar $w.i.yscroll -command \"$w.i.text yview\"");
   cmd(inter, "text $w.i.text -wrap word -width 60 -height 4 -relief sunken -yscrollcommand \"$w.i.yscroll set\"");
   cmd(inter, "pack $w.i.yscroll -side right -fill y");
   if(cur_descr->init!=NULL)
    {
     for(i=0; cur_descr->init[i]!=(char)NULL; i++)
      {
      if(cur_descr->init[i]!='[' && cur_descr->init[i]!=']' && cur_descr->init[i]!='{' && cur_descr->init[i]!='}' && cur_descr->init[i]!='\"' )
        {
         sprintf(msg, ".desc.i.text insert end \"%c\"", cur_descr->init[i]);
         cmd(inter, msg);
        }
       else
        {
         sprintf(msg, ".desc.i.text insert end \"\\%c\"", cur_descr->init[i]);
         cmd(inter, msg);
        }
   
      }
     cmd(inter, ".desc.i.text delete \"end - 1 char\"");
    }
   cmd(inter, "pack $w.i.int $w.i.text -anchor w -expand yes -fill both");
   cmd(inter, "pack $w.i -anchor w -expand yes -fill both");
   cmd(inter, "frame $w.b2");
   cmd(inter, "button $w.b2.setall -text \"Initialize\" -command {set done 11}" );
   cmd(inter, "button $w.b2.sens -text \"Sensitivity Analysis\" -command {set done 12}" );
   cmd(inter, "pack $w.b2.setall $w.b2.sens -side left -expand yes -fill both");
   cmd(inter, "pack $w.b2 -anchor w -expand yes -fill both");
   cmd(inter, "bind .desc.f.text <Control-i> {focus -force .desc.i.text}");
   cmd(inter, "bind .desc.i.text <Control-z> {set done 1}");   
   }



/*
if(cv->param==0)
 {
  cmd(inter, "button .b.eq -text Equation -command {set done 3}");
  cmd(inter, "pack .b.deb .b.sav .b.plt .b.ch .b.eq .b.descr .b.used .b.ok .b.help .b.cancel ");
 }
else
*/


Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);

cmd(inter, "frame .h -bd 2 -relief groove");
cmd(inter, "label .h.ent_var -width 30 -relief sunken -fg red -text $vname");

cmd(inter, "bind .b1 <KeyPress-Return> {set done 1}");
cmd(inter, "bind . <KeyPress-Escape> {set done 2}");

if(cv->param==0)
  cmd(inter, "label .h.lab_ent -text \"Variable\"");
if(cv->param==1)
  cmd(inter, "label .h.lab_ent -text \"Parameter\"");
if(cv->param==2)
  cmd(inter, "label .h.lab_ent -text \"Function\"");

sprintf(msg, "set obj_name %s", cv->up->label);
cmd(inter, msg);
cmd(inter, "label .h.obj -text \"in Object $obj_name\"");

cmd(inter, "button .h.prop -text \"Properties\" -command {set done 5}" );
cmd(inter, "bind .h <Double-1> {set done 5}");
cmd(inter, "bind .h.lab_ent <Double-1> {set done 5}");
cmd(inter, "bind .h.ent_var <Double-1> {set done 5}");
cmd(inter, "pack .h.lab_ent .h.ent_var .h.prop .h.obj");
cmd(inter, "pack .h .b1 .b .desc -fill x -expand yes");
cmd(inter, "focus .b1	");
cmd(inter, "bind .b1 <Control-d> {focus -force .desc.f.text}");
cmd(inter, "bind .desc.f.text <Control-z> {set done 1}");   


//cmd(inter, "wm resizable . 0 0 ");
cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
cycle_var:
set_window_size();
while(done==0)
 Tcl_DoOneEvent(0);

if(done==1)
 {
  cmd(inter, "set choice $observe");
  *choice==1?observe='y':observe='n';
  cmd(inter, "set choice $initial");
  *choice==1?initial='y':initial='n';
  cur_descr->initial=initial;
  cur_descr->observe=observe;
 }

if(done == 3)
 show_eq(lab_old, choice);
if(done == 4)
 scan_used_lab(lab_old, choice);
if(done == 7)
 scan_using_lab(lab_old, choice);
if(done == 9) 
 {
  auto_document(choice, lab_old, "ALL");
  cmd(inter, ".desc.f.text delete 1.0 end");

  for(i=0; cur_descr->text[i]!=(char)NULL; i++)
   {
   if(cur_descr->text[i]!='[' && cur_descr->text[i]!=']' && cur_descr->text[i]!='{' && cur_descr->text[i]!='}' && cur_descr->text[i]!='\"' )
     {
      sprintf(msg, ".desc.f.text insert end \"%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
    else
     {
      sprintf(msg, ".desc.f.text insert end \"\\%c\"", cur_descr->text[i]);
      cmd(inter, msg);
     }
      
   } 
  unsavedChange = true;		// signal unsaved change
 }
if(done == 7 || done == 4 || done == 3 || done == 9)
 {
  done=0;
  goto cycle_var;
 }
cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
cmd(inter, "bind . <KeyPress-d> {}");
cmd(inter, "bind . <KeyPress-s> {}");
cmd(inter, "bind . <KeyPress-p> {}");

if(done==1) 
  {
   cmd(inter, "set text_description \"[.desc.f.text get 1.0 end]\"");
   change_descr_text(lab_old);
   if(cv->param==1 || cv->num_lag>0)
    {cmd(inter, "set text_description \"[.desc.i.text get 1.0 end]\"");
     change_init_text(lab_old);
    }
  
  unsavedChange = true;		// signal unsaved change
  }
if(done==3 )
 {done=0;
  goto cycle_var;
 } 
//cmd(inter, "wm resizable . 1 1 ");

cmd(inter, "destroy .desc .b .b1 .h");
if(done==8)
 {
 return r;
 }
if(done==5||done==10)	// treat also express delete button
 {
 if(done==10)
 {
	 cmd(inter, "set answer [tk_messageBox -title \"Delete Element\" -icon warning -type okcancel -default ok -message \"Press 'Ok' to confirm deleting:\n\n$vname\"]");
	 cmd(inter, "switch -- $answer {ok {set done 1} cancel {set done 2}}");
	 if(done == 1)
	 {
		nature=3;												// simulate a name change 
		cmd(inter, "set vname \"\"; set nature 3; set numlag 0");// to empty string (delete)
  
		unsavedChange = true;		// signal unsaved change
	 }
 }
 else					// original handling of done=5
 {
 
 cv=r->search_var(NULL, lab_old);
 sprintf(msg, "set nature 3");
 cmd(inter, msg);
 sprintf(msg, "set numlag %d", cv->num_lag);
 cmd(inter, msg);
 
 cmd(inter, "frame .l");
 cmd(inter, "radiobutton .l.var -text \"Change label\" -variable nature -value 3");
 cmd(inter, "entry .l.e -width 30 -textvariable vname");
 cmd(inter, "bind .l.e <1> {.l.var invoke}");
 cmd(inter, "pack .l.var .l.e -side left");
 cmd(inter, "frame .m");
 cmd(inter, "radiobutton .m.mov -text \"Move to another Object\" -variable nature -value 4");
 sprintf(msg, "set movelabel %s", r->label);
 cmd(inter, msg);
 cmd(inter, "button .m.whe -text \"$movelabel\" -command {set nature 4; set done 3}");
 cmd(inter, "pack .m.mov .m.whe -side left");



 cmd(inter, "frame .v");
 cmd(inter, "radiobutton .v.var -text Variable -variable nature -value 0");
 cmd(inter, "label .v.l -text Lags");
 cmd(inter, "entry .v.e -width 3 -textvariable numlag");
 cmd(inter, "pack .v.var .v.l .v.e -side left");
 cmd(inter, "frame .p");
 cmd(inter, "radiobutton .p.par -text Parameter -variable nature -value 1");
 cmd(inter, "pack .p.par");
 cmd(inter, "frame .f");
 cmd(inter, "radiobutton .f.fun -text Function -variable nature -value 2");
 cmd(inter, "pack .f.fun");
 
 cmd(inter, "frame .b");
 cmd(inter, "button .b.ok -text Ok -command {set done 1}");
 cmd(inter, "button .b.esc -text Cancel -command {set done 2}");
 cmd(inter, "button .b.help -text \" Help \" -command {LsdHelp Browser.html#change_nature}");
 cmd(inter, "pack .b.ok .b.help .b.esc -side left");
 cmd(inter, "pack .l .m .v .p .f -anchor w");
 cmd(inter, "pack .b");
 cmd(inter, "bind . <KeyPress-Return> {.b.ok invoke}");
 cmd(inter, "bind .l.e <KeyPress-Return> {.b.ok invoke}");
 cmd(inter, "bind . <KeyPress-Escape> {.b.esc invoke}"); 
 done=0;
 set_window_size();
 }
 here_changenature:
 while(done==0)
  Tcl_DoOneEvent(0);
  
 if(done==3)
  {
  choose_object();
  cmd(inter, ".m.whe configure -text \"$movelabel\"");
  done=0;
  goto here_changenature; 
  }
 cmd(inter,  "destroy .l .m .v .p .b .f"); 
 cmd(inter, "bind . <KeyPress-Return> {}");
 cmd(inter, "bind . <KeyPress-Escape> {}"); 


if(done==2)
 {
  Tcl_UnlinkVar(inter, "done");
  Tcl_UnlinkVar(inter, "save");
  Tcl_UnlinkVar(inter, "savei");
  Tcl_UnlinkVar(inter, "debug");
  Tcl_UnlinkVar(inter, "plot");
  *choice=0;
  cmd(inter, "destroy .v .p .b");
  return r;

 }
  
unsavedChange = true;		// signal unsaved change

cmd(inter, "set choice $nature");
nature=*choice;

if(nature==4)
 {
  sprintf(msg, "set done [string equal $movelabel %s]", r->label);
  cmd(inter, msg);
  if(done==1)
   {
    Tcl_UnlinkVar(inter, "done");
    Tcl_UnlinkVar(inter, "save");
    Tcl_UnlinkVar(inter, "savei");
    Tcl_UnlinkVar(inter, "debug");
    Tcl_UnlinkVar(inter, "plot");
    *choice=0;
    cmd(inter, "destroy .v .p .b");
    return r;
   } 
  lab1=(char *)Tcl_GetVar(inter, "movelabel",0); 
  cv=r->search_var(NULL, lab_old);
  if(cv->param==1 || cv->num_lag>0) 
   cv->data_loaded='-';
  for(cur=root->search(lab1); cur!=NULL; cur=cur->hyper_next(cur->label) )
   cur->add_var_from_example(cv);
  cmd(inter, "set vname \"\"");
 }
cmd(inter, "set choice $numlag");
numlag=*choice;
if(nature==3 || nature ==4)
{
lab1=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(lab, lab1);
if(strcmp(lab, lab_old) )
 {
  for(cur=r; cur->up!=NULL; cur=cur->up);
  done=check_label(lab, cur);

  if(done==1)
   {cmd(inter, "tk_messageBox -title Error -icon warning -type ok -default ok -message \"The new label already exists in the model.\"");
    cmd(inter, "destroy .b .lab_ent .ent_var");
    Tcl_UnlinkVar(inter, "done");
    Tcl_UnlinkVar(inter, "save");
    Tcl_UnlinkVar(inter, "savei");
    Tcl_UnlinkVar(inter, "debug");
    Tcl_UnlinkVar(inter, "plot");
    *choice=0;
    return r;
   }
 }
 if(nature==3)
  change_descr_lab(lab_old, lab, "", "", "");
 for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
 {if(strlen(lab)!=0)
  {
	cur->chg_var_lab(lab_old, lab);
	cv=cur->search_var(NULL, lab);
	cv->save=save;
    cv->savei=savei;
	cv->debug=num==1?'d':'n';
	cv->plot=plot;
  }
  else
	{if(!strcmp(lab_old,cur->v->label))
	  {app=cur->v->next;
		//delete[strlen(cur->v->label)+1] cur->v->label;
		delete[] cur->v->label;
		//delete[cur->v->num_lag+1] cur->v->val;
		delete[] cur->v->val;
		delete cur->v;
		cur->v=app;
	  }
	 else
	 {
	 for(cur_v=cur->v; cur_v->next!=NULL; cur_v=cur_v->next)
	  {if(!strcmp(lab_old,cur_v->next->label))
		{app=cur_v->next->next;
		//delete[strlen(cur_v->next->label)+1] cur_v->next->label;
		delete[] cur_v->next->label;
		//delete[cur_v->next->num_lag+1] cur_v->next->val;
		delete[] cur_v->next->val;
		delete cur_v->next;
		cur_v->next=app;
		break;
		}
	  }
	 }
	}
 }

}
if(nature==1 || nature==0 || nature==2)
 {
 if(nature==0)
  change_descr_lab(lab_old, "", "Variable", "", "");
 if(nature==1)
  change_descr_lab(lab_old, "", "Parameter", "", "");
 if(nature==2)
  change_descr_lab(lab_old, "", "Function", "", "");


 for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
 { 
 	cv=cur->search_var(NULL, lab_old);
  cv->num_lag=numlag;
  delete[] cv->val;
  cv->val=new double[numlag+1];
  for(i=0; i<numlag+1; i++)
   cv->val[i]=0;
  cv->param=nature;
  if(cv->param==1 || cv->num_lag>0)
   cv->data_loaded='-';

 }
 }
 } 
if(done==1)
 {if(save==1 || savei==1)
   {
   for(cur=r; cur!=NULL; cur=cur->up)
    if(cur->to_compute==0)
     {
       sprintf(msg, "tk_messageBox -type ok -title Warning -message \"Warning: item\n'%s'\nset to be saved, but will not be available for the Analysis of Results, since object\n'%s'\nis set to be not computed.\"", lab_old, cur->label);
   cmd(inter, msg);
     }
   }
  for(cur=r; cur!=NULL; cur=cur->hyper_next(cur->label))
   {
  	cv=cur->search_var(NULL, lab_old);
  	cv->save=save;
    cv->savei=savei;
  	cv->debug=num==1?'d':'n';
  	cv->plot=plot;
  
   }
    
 }
if(done!=8)
  *choice=0;
else
  *choice=7;  

// do lag selection, if necessary, for initialization/sensitivity data entry
lag = 0;							// lag option for the next cases (first lag)
if ( ( done == 11 || done == 12 ) && cv->param == 0 && cv->num_lag > 1 )
{									// more than one lag to choose?
	// confirm which lag to use
	cmd( inter, "toplevel .s" );
	cmd( inter, "wm transient .s ." );
	cmd( inter, "wm title .s \"Select lag\"" );
	cmd( inter, "frame .s.i -relief groove -bd 2" );
	cmd( inter, "label .s.i.l -text \"Type the number of the lag to edit.\"" );
	cmd( inter, "set lag \"1\"" );
	cmd( inter, "entry .s.i.e -justify center -font {-weight bold} -textvariable lag" );
	cmd( inter, ".s.i.e selection range 0 end" );
	sprintf( msg, "label .s.i.w -text \"Valid values are: 1 to %d\" -fg red", cv->num_lag );
	cmd( inter, msg );
	cmd( inter, "pack .s.i.l .s.i.e .s.i.w" );
	cmd( inter, "button .s.ok -text Ok -command {set choice $lag}" );
	cmd( inter, "button .s.esc -text Cancel -command {set choice 0}" );
	cmd( inter, "pack .s.i .s.ok .s.esc -fill x");
	cmd( inter, "bind .s <KeyPress-Return> {set choice $lag}" );
	cmd( inter, "bind .s <KeyPress-Escape> {set choice 0}" );
	cmd( inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w" );
	*choice=-1;
	cmd( inter, "focus .s.i.e" );
	while ( *choice == -1 )		// wait for user action
		Tcl_DoOneEvent( 0 );
	cmd( inter, "destroy .s" );
	
	lag = abs( *choice ) - 1;	// try to extract chosed lag
	*choice = 0;
	
	// abort if necessary
	if ( lag < 0 || lag > ( cv->num_lag - 1 ) )
	{
		cmd( inter, "tk_messageBox -title \"Select lag\" -icon error -type ok -default ok -message \"Invalid lag selected.\n\nSelect a valid lag value for the variable or change the number of lagged values for this variable.\"" );
		done = 1;
	}
}

if(done==11)
 {//initialize
 *choice=0;
 set_all(choice, r, cv->label, lag);
 *choice=0;
 }

// edit sensitivity analysis data
if ( done == 12 )
{
	*choice = 0;
	bool exist = false;
	sense *cs, *ps = NULL;
	
    if (rsense==NULL)				// no sensitivity analysis structure yet?
        rsense=cs=new sense;
    else
    {
		// check if sensitivity data for the variable already exists 
		for ( cs = rsense, ps = NULL; cs != NULL; ps = cs, cs = cs->next )
			if ( ! strcmp( cs->label, cv->label ) && 
				 ( cs->param == 1 || cs->lag == lag ) )
			{
				exist = true;
				break;	// get out of the for loop
			}
			
		if ( ! exist )	// if new variable, append at the end of the list
		{
			for ( cs = rsense; cs->next != NULL; cs = cs->next );	// pick last
			cs->next = new sense;	// create new variable
			ps = cs;	// keep previous sensitivity variable
			cs = cs->next;
		}
	}
		
	if ( ! exist )		// do only for new variables in the list
	{
		cs->label = new char[ strlen( cv->label ) + 1 ];
		strcpy( cs->label, cv->label );
		cs->next = NULL;
		cs->nvalues = 0;
		cs->v = NULL;
		cs->entryOk = false;	// no valid data yet
	}
	else
		cs->entryOk = true;		// valid data already there

	// save type and specific lag in this case
	cs->param = cv->param;
	cs->lag = lag;
	
	dataentry_sensitivity( choice, cs, 0 );
	
	if ( ! cs->entryOk )		// data entry failed?
	{
		if( rsense == cs )		// is it the first variable?
			rsense = cs->next;	// update list root
		else
			ps->next = cs->next;// remove from sensitivity list		
		delete [ ] cs->label;	// garbage collection
		delete cs;
	}
	else
		unsavedSense = true;	// signal unsaved change
	*choice = 0;
}

Tcl_UnlinkVar(inter, "done");
Tcl_UnlinkVar(inter, "save");
Tcl_UnlinkVar(inter, "savei");
Tcl_UnlinkVar(inter, "debug");
Tcl_UnlinkVar(inter, "plot");

return r;


//Exit the browser and run the simulation
case 1:
cmd(inter, "destroy .m .l");
*choice=0;

if(actual_steps>0)
 {cmd(inter, "toplevel .warn");
  cmd(inter, "wm iconify .");
  cmd(inter, "wm transient .warn .");
  cmd(inter, "wm title .warn \"Warning\"");
  cmd(inter, "label .warn.l -text \"Simulation just run.\nThe configuration currently available is the last step of the previous run.\nLoad a new configuration (or re-load the previous one) to run a simulation\"");
  cmd(inter, "frame .warn.f");
  cmd(inter, "button .warn.f.ok -text \" Ok \" -command {set choice 1}");

  cmd(inter, "pack .warn.f.ok");
  cmd(inter, "pack .warn.l .warn.f");
  cmd(inter, "focus -force .warn");
  cmd(inter, "bind .warn <KeyPress-Return> {.warn.f.ok invoke}");
  cmd(inter, "bind .warn <KeyPress-Escape> {.warn.f.ok invoke}");
#ifndef DUAL_MONITOR
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
  while(*choice==0)
   Tcl_DoOneEvent(0);
  *choice=0;
  cmd(inter, "destroy .warn");
  cmd(inter, "wm deiconify .");  
  return r;
 }

if(struct_loaded==0)
 break;
cmd(inter, "button .ok -text Ok -command {set choice 1}");
cmd(inter, "button .cancel -text Cancel -command {set choice 2}");
cmd(inter, "button .help -text Help -command {LsdHelp menurun.html#run}");
sprintf(ch, "label .war1 -text \"Running the model configuration:\"");
cmd(inter, ch);
sprintf(ch, "label .war2 -text \"%s\" -fg red", simul_name);
cmd(inter, ch);
cmd(inter, "label .tosave -text \"\\n\\nYou are going to overwrite the existing configuration file\\n\"");

cmd(inter, "set overw 0"); //flag for overwriting existing total file
cmd(inter, "set dozip 0");	// flag for producing compressed files

if(sim_num>1)
{
sprintf(ch, "label .war3 -text \"Num. of Simulations: %d\"", sim_num);
cmd(inter, ch);

sprintf(ch, "label .war4 -text \"Steps for each simulation (max): %d\"", max_step);
cmd(inter, ch);

sprintf(ch, "label .war5 -text \"Result files (single simulation):\"");
cmd(inter, ch);
sprintf(ch, "label .war6 -text \"from %s_%d.res to %s_%d.res\"", simul_name, seed, simul_name, seed+sim_num-1);
cmd(inter, ch);

Tcl_LinkVar(inter, "no_res", (char *)&no_res, TCL_LINK_INT);


cmd(inter, "checkbutton .nores -text \"Skip generating result files\" -variable no_res");
cmd(inter, "checkbutton .dozip -text \"Generate zipped files\" -variable dozip");

cmd(inter, "label .war7 -text \"Total file (last steps):\"");
sprintf(ch, "label .war8 -text \"%s_%d_%d.tot\"", simul_name, seed, seed+sim_num-1);
cmd(inter, ch);
sprintf(msg, "set choice [file exist %s_%d_%d.tot] ", simul_name, seed, seed+sim_num-1);
cmd(inter, msg);

if(*choice==1)
 {
 cmd(inter, "frame .c -relief groove -bd 2");
 cmd(inter, "label .c.l -text \"Total File found\" -fg red");
 cmd(inter, "radiobutton .c.b1 -text \"Overwrite existing Total File\" -variable overw -value 0 -anchor w");
 cmd(inter, "radiobutton .c.b2 -text \"Append to existing Total File\" -variable overw -value 1 -anchor w");
 cmd(inter, "pack .c.l .c.b1 .c.b2 -fill x");
 cmd(inter, "set wind \".war1 .war2 .war3 .war4 .war5 .war6 .nores .dozip .war7 .war8 .c .tosave\"");
 }
else
 cmd(inter, "set wind \".war1 .war2 .war3 .war4 .war5 .war6 .nores .dozip .war7 .war8 .tosave\"");

cmd(inter, "foreach i $wind {pack $i}");
}
else
{
cmd(inter, "set overw 1");
cmd(inter, "label .war3 -text \"Results  will be saved in memory only\"");
cmd(inter, "set wind \".war1 .war2 .war3 .tosave\"");
cmd(inter, "foreach i $wind {pack $i}");
}

cmd(inter, "pack .ok .help .cancel");
cmd(inter, "bind . <KeyPress-Return> {.ok invoke}");
cmd(inter, "bind . <KeyPress-Escape> {.cancel invoke}");
cmd(inter, "focus -force .");
set_window_size();
*choice=0;
while(*choice==0)
 Tcl_DoOneEvent(0);

Tcl_UnlinkVar(inter, "no_res");
cmd(inter, "bind . <KeyPress-Return> {}");
cmd(inter, "bind . <KeyPress-Escape> {}");
cmd(inter, "foreach i $wind {destroy $i}");
cmd(inter, "destroy .ok .help .cancel");
if(*choice==2)
  break;

cmd(inter, "set choice $overw");
add_to_tot=*choice;
cmd(inter, "set choice $dozip");
dozip=*choice;
*choice=1;
for(n=r; n->up!=NULL; n=n->up);

blueprint->empty();			    // update blueprint to consider last changes
set_blueprint(blueprint, n);

if(strlen(path)>0)
  sprintf(struct_file, "%s/%s.lsd", path, simul_name);
else
  sprintf(struct_file, "%s.lsd", simul_name);
f=fopen(struct_file, "w");
if(f==NULL)
 {*choice=0;
  cmd(inter, "label .l1 -text Warning -fg red");
  sprintf(msg, "label .l2 -text \"File %s.lsd cannot be opened. Check if the drive or the file is set READ-ONLY\"",simul_name);
  cmd(inter, msg);
  cmd(inter, "label .l3 -text \"Press Continue to run the simulation without saving the initialization file\"");
  cmd(inter, "label .l4 -text \"Press Abort to return to the Lsd Browser\"");
  cmd(inter, "frame .b");
  cmd(inter, "button .b.c -text Continue -command {set choice 1}");
  cmd(inter, "button .b.a -text Abort -command {set choice 2}");
  cmd(inter, "bind . <KeyPress-Return> {set choice 1}");
  cmd(inter, "bind . <KeyPress-Escape> {set choice 2}");

  cmd(inter, "pack .b.c .b.a -side left");
  cmd(inter, "pack .l1 .l2 .l3 .l4 .b");
  set_window_size();
  while(*choice==0)
   Tcl_DoOneEvent(0);
  if(*choice==2)
  *choice=0;
  cmd(inter, "destroy .l1 .l2 .l3 .l4 .b");
 }
else
 {
  strcpy(lab, "");
  for(cur=r; cur->up!=NULL; cur=cur->up);
  cur->save_struct(f,lab);
  fprintf(f, "\nDATA\n");
  cur->save_param(f);
	fprintf(f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d\nEQUATION %s\n MODELREPORT %s\n", sim_num, seed, max_step, equation_name, name_rep);
  fprintf(f, "\nDESCRIPTION\n\n");
  
  
  /********************************
  ERROR: Risk saving many times the description of the same element
  for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
    {
    if(cur_descr->init==NULL)     
      fprintf(f, "%s_%s\n%s\nEND_DESCRIPTION\n\n",cur_descr->type,cur_descr->label,cur_descr->text);
    else
      fprintf(f, "%s_%s\n%s\n_INIT_\n%s\nEND_DESCRIPTION\n\n",cur_descr->type,cur_descr->label,cur_descr->text, cur_descr->init);
    } 
   **********************/ 
  save_description(cur, f);
  fprintf(f, "\nDOCUOBSERVE\n");
  for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
    {
    if(cur_descr->observe=='y')     
      fprintf(f, "%s\n",cur_descr->label);
    } 
  fprintf(f, "\nEND_DOCUOBSERVE\n\n");
  fprintf(f, "\nDOCUINITIAL\n");
  for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
    {
    if(cur_descr->initial=='y')     
      fprintf(f, "%s\n",cur_descr->label);
    } 
  fprintf(f, "\nEND_DOCUINITIAL\n\n");
  save_eqfile(f);
  fclose(f);
 }
/*****
cmd(inter, "set choice $check_optimized");
check_optimized=*choice;
cmd(inter, "set choice $optimized");
optimized=*choice;
******/
*choice=1; 

return(n);



//Exit Lsd
case 11:
    cmd( inter, "set exit [ tk_messageBox -title \"Exit?\" -type okcancel -default ok -icon info -message \"You are closing Lsd.\\n\\nAll data generated and not saved will be lost.\" ]; if { $exit == \"ok\" } { set choice 1 } { set choice 2 }" ); 

	if(*choice==1 && discard_change( ) )	// unsaved configuration changes ?
		myexit(0);
	cmd(inter, "destroy .m .l");
	*choice=0;
   break;


//Change simulation name. No active in this version, being substitute with save
/*
case 16:
Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
sprintf(lab, "set res %s", simul_name);
cmd(inter, lab);
cmd(inter, "destroy .l .m");
cmd(inter, "frame .d");
cmd(inter, "button .ok -text Ok -command {set done 1}");
cmd(inter, "button .cancel -text Cancel -command {set done 2}");
cmd(inter, "entry .d.ent_var -width 20 -relief sunken -textvariable res");
cmd(inter, ".d.ent_var selection range 0 end");
cmd(inter, "label .d.lab_ent -text \"New Simulation name: \"");
sprintf(msg, "button .search -text \"Browse\" -command {set tk_strictMotif 0; set bah [tk_getSaveFile -title \"Browse Lsd Files\" -initialdir $path -filetypes {{{Lsd Model Files} {.lsd}} {{All Files} {*}} }];set tk_strictMotif 1; if {[string length $bah] > 0} {set res $bah} {set done 2}; .d.ent_var selection range 0 end; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .lsd $res] -1];set res [string range $res 0 $last]}");
cmd(inter, msg);
cmd(inter, "label .d.lab_path -text Path");
cmd(inter, "entry .d.ent_path -width 60 -relief sunken -textvariable path");
if(strlen(path)>0)
 sprintf(msg, "set path \"%s\"", path);
else
 sprintf(msg, "set path [pwd]");
cmd(inter, msg);
cmd(inter, "pack .d.lab_ent .d.ent_var .d.lab_path .d.ent_path -anchor w");
cmd(inter, "pack .d .search .ok .cancel");
cmd(inter, "focus .d.ent_var");
cmd(inter, "bind . <KeyPress-Return> {set done 1}");
cmd(inter, "bind . <KeyPress-Escape> {set done 2}");

set_window_size();
while(done==0)
 Tcl_DoOneEvent(0);
cmd(inter, "destroy .ok .cancel .search .d");
Tcl_UnlinkVar(inter, "done");

lab1=(char *)Tcl_GetVar(inter, "bah",0);
strcpy(lab, lab1);
printf("%s", lab);

if(done==2)
 break;
lab1=(char *)Tcl_GetVar(inter, "res",0);
strcpy(lab, lab1);

if(strlen(lab)==0)
 break;
delete[] simul_name;

simul_name=new char[strlen(lab)+1];
strcpy(simul_name, lab);
lab1=(char *)Tcl_GetVar(inter, "path",0);
strcpy(msg, lab1);
if(strlen(msg)>0)
 {delete[] path;
  path =new char[strlen(msg)+1];
  strcpy(path, msg);
  sprintf(msg, "%s", path);
  i=cd( msg);
  printf("%d",i);
 }
else
 strcpy(path,"");


delete[] struct_file;
if(strlen(path)>0)
 {struct_file=new char[strlen(path)+strlen(simul_name)+6];
  sprintf(struct_file,"%s/%s.lsd",path,simul_name);
 }
else
 {struct_file=new char[strlen(simul_name)+6];
  sprintf(struct_file,"%s.lsd",simul_name);
 }

break;
*/

//Load a model
case 17:
case 38: //quick reload
   cmd(inter, "destroy .l .m");

   if(struct_loaded==1)
	  { 
	 if ( ! discard_change( ) )		// unsaved configuration?
		 break;

     if(*choice==17)
      {  
       cmd(inter, "button .ok -text Empty -command {set choice 1}");
       cmd(inter,  "label .war -text \"WARNING: a model is already loaded in memory \\nPress Empty to remove the old model \\nor\\nCancel\"");
       cmd(inter, "button .canc -text Cancel -command {set choice 2}");
       cmd(inter, "pack .war .ok .canc");
       cmd(inter, "bind . <KeyPress-Return> {set choice 1}");
       cmd(inter, "bind . <KeyPress-Cancel> {set choice 2}");
       set_window_size();
	    while(*choice==0)
  		   Tcl_DoOneEvent(0);
	    cmd(inter, "destroy .war .ok .canc");
       if(*choice==2)
        {
         break;
        }
       } 

       cmd(inter, "set a [split [winfo children .] ]");  // remove old runtime plots
       cmd(inter, " foreach i $a {if [string match .plt* $i] {destroy $i}}");
       for(n=r; n->up!=NULL; n=n->up);
//		  cmd(inter, "destroy .l .b");
		  n->empty();
      empty_cemetery();
      lsd_eq_file[0]=(char)NULL;
		  n->label = new char[strlen("Root")+1];
		  strcpy(n->label, "Root");
		  r=n;
		  cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
      empty_descr();
      add_description("Root", "Object", "(no description available)");
	  empty_sensitivity(rsense); 	// discard sensitivity analysis data
	  rsense=NULL;
	  unsavedSense = false;			// nothing to save
 	  findexSens=0;
	  nodesSerial=0;				// network node serial number global counter
    }

	actual_steps=0;					//Flag that no simulation has been run

if(*choice==17)
{
  *choice=0;
  strcpy(lastObj,"");	// disable last object for quick reload
  sprintf(lab, "set res %s", simul_name);
  cmd(inter, lab);
  if(strlen(path)>0)
   sprintf(msg, "set path \"%s\"", path);
  else
   sprintf(msg, "set path [pwd]");
  cmd(inter, msg);
  cmd(inter, "cd $path");
  //sprintf(msg, "set tk_strictMotif 0; set bah [tk_getOpenFile -initialdir $path -title \"Load Lsd Files\" -defaultextension \".lsd\" -initialfile $res  -filetypes {{{Lsd Model Files} {.lsd}} {{All Files} {*}} }];set tk_strictMotif 1");
  
  cmd(inter, "set a \"\"");
  sprintf(msg, " set bah [tk_getOpenFile  -defaultextension \".lsd\" -initialdir $path  -filetypes {{{Lsd Model Files} {.lsd}}  }]");
  //sprintf(msg, "set bah [tk_getSaveFile -title \"Save Lsd Model\" -defaultextension \".lsd\" -initialfile $res -initialdir [pwd] -filetypes {{{Lsd Model Files} {.lsd}} {{All Files} {*}} }]");

  cmd(inter, msg);
  
  
  cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .lsd $res] -1];set res [string range $res 0 $last]} {set choice 2}");
  if(*choice==2)
   {
    break;
   }
  lab1=(char *)Tcl_GetVar(inter, "res",0);
  strcpy(lab, lab1);
  
  if(strlen(lab)==0)
   break;
  delete[] simul_name;
  simul_name=new char[strlen(lab)+1];
  strcpy(simul_name, lab);
  lab1=(char *)Tcl_GetVar(inter, "path",0);
  strcpy(msg, lab1);
  if(strlen(msg)>0)
   {delete[] path;
    path =new char[strlen(msg)+1];
    strcpy(path, msg);
    sprintf(msg, "cd %s", path);
    cmd(inter, "cd $path");
//    i=cd( msg);//Questionable
  //  printf("%d",i);
   }
  else
   strcpy(path,"");
  delete[] struct_file;
  if(strlen(path)>0)
   {struct_file=new char[strlen(path)+strlen(simul_name)+6];
    sprintf(struct_file,"%s/%s.lsd",path,simul_name);
   }
  else
   {struct_file=new char[strlen(simul_name)+6];
    sprintf(struct_file,"%s.lsd",simul_name);
   }
 } 
 

	f=fopen(struct_file, "r");
	if(f==NULL)
	 {
     cmd(inter, "button .ok -text Ok -command {set choice 1}");
     if(strlen(path)>0)
	   sprintf(msg, "label .war -text \"WARNING: file for model \\n %s \\nnot found in directory %s\"", simul_name, path);
     else
       sprintf(msg, "label .war -text \"WARNING: file for model \\n %s \\nnot found in current directory\"", simul_name);
 	  cmd(inter, msg);
	  cmd(inter, "pack .war .ok");
     cmd(inter, "bind . <KeyPress-Return> {set choice 1}");
     set_window_size();
	  while(*choice==0)
		 Tcl_DoOneEvent(0);
	  cmd(inter, "destroy .war .ok");
     break;
    }
	else
	 {
    cmd(inter, "catch {unset ModElem}");
    r->load_struct(f);
	  struct_loaded=1;
	  //fclose(f);
	  //f=NULL;
    fscanf(f, "%s", msg); //should be DATA
	  r->load_param(struct_file, 1,f);
	  show_graph(r);
	 //f=search_str(struct_file, "SIM_NUM");
	 //if(f!=NULL)
   fscanf(f, "%s",msg); //should be SIM_NUM 
	  fscanf(f, "%d", &sim_num);
	 /*
   fclose(f);
	 f=search_str(struct_file, "SEED");
	 if(f!=NULL) **/
   fscanf(f, "%s",msg); //should be SEED
	  fscanf(f, "%d", &seed);

/*	 fclose(f);
	 f=search_str(struct_file, "MAX_STEP");
	 if(f!=NULL) **/
   fscanf(f, "%s",msg); //should be MAX_STEP
   fscanf(f, "%d", &max_step);
/*	 fclose(f);
	 f=search_str(struct_file, "EQUATION");
	 if(f!=NULL) **/
   fscanf(f, "%s",msg); //should be EQUATION
   fgets(msg, 200, f);

	 delete[] equation_name;
	 equation_name=new char[strlen(msg)+1];
	 strcpy(equation_name, msg+1);
    if(equation_name[strlen(equation_name)-1]=='\n')
      equation_name[strlen(equation_name)-1]=(char)NULL;
    if(equation_name[strlen(equation_name)-1]=='\r')
      equation_name[strlen(equation_name)-1]=(char)NULL;
  
	 //f=search_str(struct_file, "MODELREPORT");
   if(fscanf(f, "%s",msg)!=1) //should be MODELREPORT
    {
    fclose(f);
    autofill_descr(r);
    *choice=0;
    t=0;
    goto end1738;
    }  
   fscanf(f, "%s", name_rep);
   if(fscanf(f, "%s", msg)!=1) //should be DESCRIPTION
    {
    fclose(f);
    autofill_descr(r);
    *choice=0;
    t=0;
    goto end1738;
    }  
   
   fscanf(f, "%s", msg); //should be the first description   
   i=1;
   while(strcmp(msg, "DOCUOBSERVE")!=0 && i==1)
    { 
     load_description(f);
     if(fscanf(f, "%s", msg)!=1) 
      i=0;
    }
   if(i==0)
    {
    fclose(f);
    *choice=0;
    t=0;
    goto end1738;
    } 
   fscanf(f, "%s", msg);  
   while(strcmp(msg, "END_DOCUOBSERVE")!=0)
    {
    cur_descr=search_description(msg);
    if(cur_descr==NULL)
    {
     sprintf(msg, "Warning! description for '%s' not found. Check elements to observe.\\n", msg);
     plog(msg);
     fscanf(f, "%s", msg);
    }
    else 
     {
      cur_descr->observe='y';
      fscanf(f, "%s", msg);
     } 
    }
   fscanf(f, "%s", msg);  
   fscanf(f, "%s", msg);  
   while(strcmp(msg, "END_DOCUINITIAL")!=0)
    {
    cur_descr=search_description(msg);
    if(cur_descr==NULL)
    {
     sprintf(msg, "Warning! description for '%s' not found. Check elements to initialize.\\n", msg);
     plog(msg);
     fscanf(f, "%s", msg);
    }
    else
     {
     cur_descr->initial='y';
     fscanf(f, "%s", msg);
     }
    }
   if(fscanf(f, "%s", msg)==1)
    {//here is the eq_file
    fscanf(f, "%s", msg);
    strcpy(lsd_eq_file, msg);
    while( fgets(msg, 1000, f)!=NULL && strncmp(msg, "END_EQ_FILE", 11) )
      strcat(lsd_eq_file, msg);
    i=strlen(lsd_eq_file);
    lsd_eq_file[i-1]=lsd_eq_file[i];  
    if(ignore_eq_file==0 && strcmp(lsd_eq_file, eq_file)!=0)
     {
      cmd(inter, "tk_messageBox -type ok -icon warning -title \"Warning\" -message \"Warning.\\nThe configuration file loaded has been previously run with equations different from those used to create the Lsd model program.\\nThe changes may affect the simulation results. You can offload the original equations in a new equation file and compare differences using TkDiff in LMM (menu Edit).\"");

     }  
    }  
	unsavedChange = false;		// no changes to save
    fclose(f);
	 }
   
   *choice=0;
   t=0;
end1738:
	if(strlen(lastObj)>0)
	{
		for(n=r; n->up!=NULL; n=n->up);
		n=n->search(lastObj);
		return n;
	}
	break;

//Save a model
case 73:
case 18:
	saveAs = ( *choice == 73 ) ? true : false;

	Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
   cmd(inter, "destroy .l .m");

   if(struct_loaded==0)
	  { cmd(inter, "button .ok -text Empty -command {set done 1}");
       cmd(inter,  "label .war -text \"WARNING: no model to save \\nPress Empty to save an empty model \\nor\\nCancel to return\"");
       cmd(inter, "button .canc -text Cancel -command {set done 2}");
       cmd(inter, "pack .war .ok .canc");
       cmd(inter, "bind . <KeyPress-Return> {set done 1}");
       cmd(inter, "bind . <KeyPress-Cancel> {set done 2}");
       set_window_size();
	    while(done==0)
		  Tcl_DoOneEvent(0);
	    cmd(inter, "destroy .war .ok .canc");
       if(done==2)
        {Tcl_UnlinkVar(inter, "done");
         break;
        }
		saveAs = true;	// require file name to save
     }
   if(actual_steps>0)
	  { cmd(inter, "button .ok -text Save -command {set done 1}");
       cmd(inter,  "label .war -text \"WARNING: The presently loaded model is the final state of a simulation run\\nPress Save to save is anyway \\nor\\nCancel to abort saving\"");
       cmd(inter, "button .canc -text Cancel -command {set done 2}");
       cmd(inter, "pack .war .ok .canc");
       cmd(inter, "bind . <KeyPress-Return> {set done 1}");
       cmd(inter, "bind . <KeyPress-Cancel> {set done 2}");
       set_window_size();
	    while(done==0)
		  Tcl_DoOneEvent(0);
	    cmd(inter, "destroy .war .ok .canc");
       if(done==2)
        {Tcl_UnlinkVar(inter, "done");
         break;
        }
		saveAs = true;	// require file name to save
     }

done=0;
sprintf(lab, "set res %s", simul_name);
cmd(inter, lab);
if(strlen(path)>0)
 sprintf(msg, "set path \"%s\"", path);
else
 sprintf(msg, "set path [pwd]");
cmd(inter, msg);

if ( saveAs )			// only asks file name if instructed to or necessary
{
//sprintf(msg, "set tk_strictMotif 0; set bah [tk_getSaveFile -title \"Save Lsd Model\" -defaultextension \".lsd\" -initialfile $res -initialdir [pwd] -filetypes {{{Lsd Model Files} {.lsd}} {{All Files} {*}} }]; set tk_strictMotif 1");
sprintf(msg, "set bah [tk_getSaveFile -title \"Save Lsd Model\" -defaultextension \".lsd\" -initialfile $res -initialdir [pwd] -filetypes {{{Lsd Model Files} {.lsd}} {{All Files} {*}} }]");
cmd(inter, msg);

cmd(inter, "set res $bah");

cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .lsd $res] -1];if {$last > 0} {set res [string range $res 0 $last]} {}} {set done 2}");
if(done==2)
 {Tcl_UnlinkVar(inter, "done");
  break;
 }
lab1=(char *)Tcl_GetVar(inter, "res",0);
strcpy(lab, lab1);

if(strlen(lab)==0)
 break;
delete[] simul_name;
simul_name=new char[strlen(lab)+1];
strcpy(simul_name, lab);
lab1=(char *)Tcl_GetVar(inter, "path",0);
strcpy(msg, lab1);
if(strlen(msg)>0)
 {delete[] path;
  path =new char[strlen(msg)+1];
  strcpy(path, msg);
  sprintf(msg, "%s", path);
  cmd(inter, "pwd");
  cmd(inter, "cd $path");
  cmd(inter, "pwd");
//  i=cd( msg);
//  printf("%d",i);
 }
else
 strcpy(path,"");
delete[] struct_file;
if(strlen(path)>0)
 {struct_file=new char[strlen(path)+strlen(simul_name)+6];
  sprintf(struct_file,"%s/%s.lsd",path,simul_name);
 }
else
 {struct_file=new char[strlen(simul_name)+6];
  sprintf(struct_file,"%s.lsd",simul_name);
 }
}	// end if ( saveAs )

f=fopen(struct_file, "w");
if(f==NULL)
 {*choice=0;
  cmd(inter, "label .l1 -text Warning -fg red");
  sprintf(msg, "label .l2 -text \"File %s.lsd cannot be opened for writing. Check if the drive or the file is set READ-ONLY\"",simul_name);
  cmd(inter, msg);
  cmd(inter, "label .l3 -text \"The model is NOT saved! Change file name or select a drive with write permission\"");
  cmd(inter, "button .b -text Ok -command {set choice 1}");
  cmd(inter, "pack .l1 .l2 .l3 .b");
  cmd(inter, "bind . <KeyPress-Return> {set choice 1}");
  set_window_size();
  while(*choice==0)
   Tcl_DoOneEvent(0);

  *choice=0;
  cmd(inter, "destroy .l1 .l2 .l3 .b");
 }
else
 {
	strcpy(lab, "");
  cur=r;
	for(; cur->up!=NULL; cur=cur->up);
	cur->save_struct(f,lab);
   fprintf(f, "\n\nDATA");
	for(cur=r; cur->up!=NULL; cur=cur->up);
	cur->save_param(f);
	fprintf(f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d\nEQUATION %s\nMODELREPORT %s\n", sim_num, seed, max_step, equation_name, name_rep);
  fprintf(f, "\nDESCRIPTION\n\n");
  /*********************
  for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
    {
    if(cur_descr->init==NULL)     
      fprintf(f, "%s_%s\n%s\nEND_DESCRIPTION\n\n",cur_descr->type,cur_descr->label,cur_descr->text);
    else
      fprintf(f, "%s_%s\n%s\n_INIT_\n%s\nEND_DESCRIPTION\n\n",cur_descr->type,cur_descr->label,cur_descr->text, cur_descr->init);
    } 
  **********************/
  save_description(cur, f);  
  fprintf(f, "\nDOCUOBSERVE\n");
  for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
    {
    if(cur_descr->observe=='y')     
      fprintf(f, "%s\n",cur_descr->label);
    } 
  fprintf(f, "\nEND_DOCUOBSERVE\n\n");
  fprintf(f, "\nDOCUINITIAL\n");
  for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
    {
    if(cur_descr->initial=='y')     
      fprintf(f, "%s\n",cur_descr->label);
    } 
  fprintf(f, "\nEND_DOCUINITIAL\n\n");
  save_eqfile(f);
  
	fclose(f);
 }  
   unsavedChange = false;		// signal no unsaved change
   Tcl_UnlinkVar(inter, "done");
	break;


//Edit Objects' numbers
case 19:
*choice=0;
cmd(inter, "destroy .l .m");
if(actual_steps>0)
 {cmd(inter, "toplevel .warn");
  cmd(inter, "wm transient .warn .");
  cmd(inter, "wm title .warn \"Error\"");
  cmd(inter, "label .warn.l1 -text \"Error\" -foreground red");
  cmd(inter, "label .warn.l -text \"Simulation already run\nLoad again the model to make changes to the Object's numbers\"");
  cmd(inter, "button .warn.b -text Ok -command {set choice 1}");
  cmd(inter, "button .warn.help -text Help -command {LsdHelp QuickHelp.html#problem}");
  cmd(inter, "pack .warn.l1 .warn.l .warn.b .warn.help");
  cmd(inter, "focus -force .warn");
  cmd(inter, "bind .warn <KeyPress-Return> {.warn.b invoke}");
#ifndef DUAL_MONITOR
 cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
 cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
  while(*choice==0)
   Tcl_DoOneEvent(0);
  *choice=0;
  cmd(inter, "destroy .warn");
  return r;
 }

        for(n=r; n->up!=NULL; n=n->up);

		  *choice=0;
		  strcpy(lab, r->label);
cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
		  set_obj_number(n, choice);
cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
		  r=n->search(lab);
		  unsavedChange = true;		// signal unsaved change
		  *choice=0;
		  break;

//Edit initial values for Objects currently  pointed by the browser
case 21:

*choice=0;
cmd(inter, "destroy .l .m");
if(actual_steps>0)
 {cmd(inter, "toplevel .warn");
  cmd(inter, "wm title .warn \"Error\"");
  cmd(inter, "wm transient .warn .");
  cmd(inter, "label .warn.l1 -text \"Error\" -foreground red");
  cmd(inter, "label .warn.l -text \"Simulation already run\nLoad again the model to make changes to the initial values\"");
  cmd(inter, "button .warn.b -text Ok -command {set choice 1}");
  cmd(inter, "button .warn.help -text Help -command {LsdHelp QuickHelp.html#problem}");
  cmd(inter, "pack .warn.l1 .warn.l .warn.b .warn.help");
  cmd(inter, "focus -force .warn");
  cmd(inter, "bind .warn <KeyPress-Return> {.warn.b invoke}");
#ifndef DUAL_MONITOR
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
  while(*choice==0)
   Tcl_DoOneEvent(0);
  *choice=0;
  cmd(inter, "destroy .warn");
  return r;
 }


cmd(inter, "bind . <KeyPress-Return> {}");

for(n=r; n->up!=NULL; n=n->up);
cmd(inter, "destroy .l .m");


cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
edit_data(n, choice, r->label);
cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
unsavedChange = true;		// signal unsaved change
*choice=0;
break;

//Empty the model
case 20:
      cmd(inter, "destroy .l .m");
	  if ( ! discard_change( ) )	// check for unsaved configuration changes
	  {
		*choice = 0;
		break;
	  }
	  
	  cmd( inter, "if { [ winfo exists .model_str ] == 1 } { wm withdraw .model_str }");
	  
      for(n=r; n->up!=NULL; n=n->up);
		  cmd(inter, "destroy .l .m");
		  n->empty();
      empty_cemetery();
      lsd_eq_file[0]=(char)NULL;
		  n->label = new char[strlen("Root")+1];
		  strcpy(n->label, "Root");
		  r=n;
		  cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
		  *choice=0;
	  strcpy(lastObj,"");	// disable last object for quick reload
      actual_steps=0;
      empty_descr();
	  empty_sensitivity(rsense); 	// discard sensitivity analysis data
	  rsense=NULL;
	  unsavedSense = false;			// nothing to save
 	  findexSens=0;
	  nodesSerial=0;				// network node serial number global counter
      add_description("Root", "Object", "(no description available)");      
      cmd(inter, "catch {unset ModElem}");
	  unsavedChange = false;		// signal no unsaved change
      break;

//Simulation manager: sets seeds, number of steps, number of simulations
case 22:
		  cmd(inter, "destroy .l .m");
  		  *choice=0;
		  // save previous values to allow canceling operation
		  temp[1] = sim_num; 
		  temp[2] = seed; 
		  temp[3] = max_step; 
		  temp[4] = when_debug;
		  Tcl_LinkVar(inter, "sim_num", (char *) &sim_num, TCL_LINK_INT);
		  Tcl_LinkVar(inter, "seed", (char *) &seed, TCL_LINK_INT);
		  Tcl_LinkVar(inter, "max_step", (char *) &max_step, TCL_LINK_INT);
        cmd(inter, "label .tit -text \"Set Simulation Settings\" -font {System 14 bold}");
        cmd(inter, "frame .f -relief groove -bd 2");
        cmd(inter, "frame .f.a -bd 2");
		  cmd(inter, "label .f.a.l -width 25 -text \"Number of Simulations\"");
		  cmd(inter, "entry .f.a.e -textvariable sim_num -width 5");
		  cmd(inter, ".f.a.e selection range 0 end");
        cmd(inter, "pack .f.a.l .f.a.e -side left -anchor w");
        cmd(inter, "frame .f.b -bd 2");
		  cmd(inter, "label .f.b.l1 -width 25 -text \"Initial Seed\"");
		  cmd(inter, "entry .f.b.e1 -textvariable seed -width 5");
		  cmd(inter, ".f.b.e1 selection range 0 end");
        cmd(inter, "pack .f.b.l1 .f.b.e1 -side left -anchor w");
        cmd(inter, "frame .f.c -bd 2");
		  cmd(inter, "label .f.c.l2 -width 25 -text \"Simulation Steps\"");
		  cmd(inter, "entry .f.c.e2 -textvariable max_step -width 8");
		  cmd(inter, ".f.c.e2 selection range 0 end");
        cmd(inter, "pack .f.c.l2 .f.c.e2 -side left -anchor w");

        cmd(inter, "frame .f.d -bd 2");
		  cmd(inter, "label .f.d.l2 -width 25 -text \"Insert Debugger at:\"");
		  cmd(inter, "entry .f.d.e2 -textvariable when_debug -width 8");
		  cmd(inter, ".f.d.e2 selection range 0 end");
        cmd(inter, "pack .f.d.l2 .f.d.e2 -side left -anchor w");

        cmd(inter, "frame .f.e -bd 2");
		  cmd(inter, "label .f.e.l2 -width 25 -text \"Print until stack:\"");
       sprintf(msg, "set stack_info %d", stackinfo_flag);
        cmd(inter, msg);
		  cmd(inter, "entry .f.e.e2 -textvariable stack_info -width 8");
		  cmd(inter, ".f.e.e2 selection range 0 end");
        cmd(inter, "pack .f.e.l2 .f.e.e2 -side left -anchor w");

		  cmd(inter, "frame .b");
		  cmd(inter, "button .b.ok -text \" Ok \" -command {set choice 1}");
		  cmd(inter, "button .b.esc -text \" Cancel \" -command {set choice 2}");
		  cmd(inter, "button .b.help -text \" Help \" -command {LsdHelp menurun.html#simsetting}");
		  cmd(inter, "pack .b.ok .b.help .b.esc -side left");
        cmd(inter, "pack .f.a .f.b .f.c .f.d .f.e -anchor w");
		  cmd(inter, "pack .tit .f .b ");
        cmd(inter, "focus .f.a.e");
        cmd(inter, "bind .f.a.e <KeyPress-Return> {focus .f.b.e1; .f.b.e1 selection range 0 end}");
        cmd(inter, "bind .f.b.e1 <KeyPress-Return> {focus .f.c.e2; .f.c.e2 selection range 0 end}");
        cmd(inter, "bind .f.c.e2 <KeyPress-Return> {focus .f.d.e2; .f.d.e2 selection range 0 end}");
        cmd(inter, "bind .f.d.e2 <KeyPress-Return> {focus .f.e.e2; .f.e.e2 selection range 0 end}");
        cmd(inter, "bind .f.e.e2 <KeyPress-Return>  {focus .b.ok}");
        cmd(inter, "bind .b.ok <KeyPress-Return> {set choice 1}");
        cmd(inter, "bind .b.esc <KeyPress-Escape> {set choice 2}");

     set_window_size();
		  while(*choice==0)
			 Tcl_DoOneEvent(0);
		 
	 cmd(inter, "destroy .tit .f .b");
	 
	 if ( *choice == 2 )	// Escape - revert previous values
	 {
		sim_num = temp[1];
		seed = temp[2];
		max_step = temp[3];
		when_debug = temp[4];
		*choice=0;
		break;
	 }

       cmd(inter, "set choice $stack_info");
       stackinfo_flag=*choice;

		  Tcl_UnlinkVar(inter, "seed");
		  Tcl_UnlinkVar(inter, "sim_num");
//       plog("$stack_info");
		  unsavedChange = true;		// signal unsaved change
		  *choice=0;
		  break;

//Move browser to Object pointed on the graphical model map
case 24:
			cmd(inter, "destroy .l .m");
		  *choice=0;

		    if(res_g==NULL)
			  return r;
			for(n=r; n->up!=NULL; n=n->up);
			n=n->search(res_g);
			strcpy(lastObj,res_g);	// save last object for quick reload
			return n;

//Edit initial values of Objects pointed on the graphical map
case 25:
*choice=0;
cmd(inter, "destroy .l .m");
if(actual_steps>0)
 {cmd(inter, "toplevel .warn");
  cmd(inter, "wm title .warn \"Error\"");
  cmd(inter, "wm transient .warn .");
  cmd(inter, "label .warn.l1 -text \"Error\" -foreground red");
  cmd(inter, "label .warn.l -text \"Simulation already run\nLoad again the model to make changes to the initial values\"");
  cmd(inter, "button .warn.b -text Ok -command {set choice 1}");
  cmd(inter, "button .warn.help -text Help -command {LsdHelp QuickHelp.html#problem}");
  cmd(inter, "pack .warn.l1 .warn.l .warn.b .warn.help");
  cmd(inter, "focus -force .warn");
  cmd(inter, "bind .warn <KeyPress-Return> {.warn.b invoke}");
#ifndef DUAL_MONITOR
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
  while(*choice==0)
   Tcl_DoOneEvent(0);
  *choice=0;
  cmd(inter, "destroy .warn");
  return r;
 }

		    if(res_g==NULL)
			  return r;
			for(n=r; n->up!=NULL; n=n->up);
			r=n->search(res_g);
      cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
			edit_data(n, choice, r->label);

cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
			unsavedChange = true;		// signal unsaved change
			*choice=choice_g=0;
			return r;

//Enter the analysis of results module
case 26:
			cmd(inter, "destroy .l .m");
			analysis(choice);
			*choice=0;
			return r;

//Remove all the debugging flags
case 27:
			cmd(inter, "destroy .l .m");
         *choice=0;
			cmd(inter, "frame .w");
			cmd(inter, "label .w.b -text \"Confirm the removal of all debugging information?\"");
			cmd(inter, "button .w.ok -text Ok -command {set choice 1}");
			cmd(inter, "button .w.can -text Cancel -command {set choice 2}");
			cmd(inter, "pack .w.b .w.ok .w.can");
			cmd(inter, "pack .w");
			cmd(inter, "bind . <KeyPress-Return> {.w.ok invoke}");
			cmd(inter, "bind . <KeyPress-Escape> {.w.can invoke}");
      set_window_size();
			while(*choice==0)
			 Tcl_DoOneEvent(0);
			cmd(inter, "destroy .w");

			if(*choice==1)
			{
			for(n=r; n->up!=NULL; n=n->up);
			clean_debug(n);
			unsavedChange = true;		// signal unsaved change
			}
			*choice=0;
			return r;

//Change Equation File from which to take the code to show
case 28:
Tcl_LinkVar(inter, "done", (char *) &done, TCL_LINK_INT);
cmd(inter, "destroy .l .m");
cmd(inter, "button .ok -text Ok -command {set done 1}");
cmd(inter, "button .cancel -text Cancel -command {set done 2}");
cmd(inter, "button .help -text Help -command {LsdHelp menumodel.html#setequation}");

sprintf(msg, "set res \"%s\"", equation_name);
cmd(inter, msg);

cmd(inter, "entry .ent_var -width 20 -relief sunken -textvariable res");
cmd(inter, ".ent_var selection range 0 end");
cmd(inter, "bind .ent_var <KeyPress-Return> {set done 1}");
cmd(inter, "bind . <KeyPress-Return> {set done 1}");
cmd(inter, "bind . <KeyPress-Escape> {set done 2}");

cmd(inter, "label .lab_ent -text \"New Equation file name: \"");
cmd(inter, "button .search -text \"Search File\" -command {set res [file tail [tk_getOpenFile -initialdir [pwd] -filetypes {{{Lsd Equation Files} {.cpp}} {{All Files} {*}} }]]; .ent_var selection range 0 end}");

cmd(inter, "pack .lab_ent .ent_var .search .ok .help .cancel");
cmd(inter, "focus .ent_var");
set_window_size();
while(done==0)
 Tcl_DoOneEvent(0);

if(done==1)
{
lab1=(char *)Tcl_GetVar(inter, "res",0);
strcpy(lab, lab1);
delete[] equation_name;
equation_name=new char[strlen(lab)+1];
strcpy(equation_name, lab);
unsavedChange = true;		// signal unsaved change
}
cmd(inter, "destroy .ok .cancel .lab_ent .search .help .ent_var");
Tcl_UnlinkVar(inter, "done");
break;

//Shortcut to show Info window (former equation). REMOVED
case 29:
cmd(inter, "destroy .m .l");

lab1=(char *)Tcl_GetVar(inter, "vname",0);
sscanf(lab1, "%s", lab_old);
sprintf(msg, "set vname %s",lab_old);
cmd(inter, msg);
//show_description(lab_old);
 show_eq(lab_old, choice);
*choice=0;
return r;


//Remove all the save flags
case 30:

			cmd(inter, "destroy .l .m");
         *choice=0;
			cmd(inter, "frame .w");
			cmd(inter, "label .w.b -text \"Confirm the removal of all saving information?\\nNo data will be saved\"");
			cmd(inter, "button .w.ok -text Ok -command {set choice 1}");
			cmd(inter, "button .w.can -text Cancel -command {set choice 2}");
			cmd(inter, "pack .w.b .w.ok .w.can");
			cmd(inter, "pack .w");
			cmd(inter, "bind . <KeyPress-Return> {.w.ok invoke}");
			cmd(inter, "bind . <KeyPress-Escape> {.w.can invoke}");
      set_window_size();
			while(*choice==0)
			 Tcl_DoOneEvent(0);

			cmd(inter, "destroy .w");

			if(*choice==1)
			{
			for(n=r; n->up!=NULL; n=n->up);
			clean_save(n);
			unsavedChange = true;		// signal unsaved change
			}
			*choice=0;
			return r;
			break;

//Show variables to be saved
case 39:
			cmd(inter, "destroy .l .m");
         *choice=50;
			for(n=r; n->up!=NULL; n=n->up);
         plog("\n\nVariables and Parameters saved:\n");
			show_save(n);
      cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
			return r;
			break;
//Show variables to be observed
case 42:
			cmd(inter, "destroy .l .m");
         *choice=50;
			for(n=r; n->up!=NULL; n=n->up);
         plog("\n\nVariables and Parameters containing results:\n");
			show_observe(n);
      cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
			return r;
			break;

//Show variables to be initialized
case 49:
			cmd(inter, "destroy .l .m");
         *choice=50;
			for(n=r; n->up!=NULL; n=n->up);
         plog("\n\nVariables and Parameters relevant to initialize:\n");
			show_initial(n);
      cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
			return r;
			break;


//Remove Runtime Plots
case 40:
			cmd(inter, "destroy .l .m");
         *choice=0;
         cmd(inter, "set a [split [winfo children .] ]");
         cmd(inter, " foreach i $a {if [string match .plt* $i] {destroy $i}}");
			return r;
			break;



//Remove all the plot flags
case 31:

			cmd(inter, "destroy .l .m");
         *choice=0;
			cmd(inter, "frame .w");
			cmd(inter, "label .w.b -text \"Confirm the removal of all plotting information?\\nNo data will be plotted\"");
			cmd(inter, "button .w.ok -text Ok -command {set choice 1}");
			cmd(inter, "button .w.can -text Cancel -command {set choice 2}");
			cmd(inter, "pack .w.b .w.ok .w.can");
			cmd(inter, "pack .w");
			cmd(inter, "bind . <KeyPress-Return> {.w.ok invoke}");
			cmd(inter, "bind . <KeyPress-Escape> {.w.can invoke}");
      set_window_size();
			while(*choice==0)
			 Tcl_DoOneEvent(0);

			cmd(inter, "destroy .w");

			if(*choice==1)
			{
			for(n=r; n->up!=NULL; n=n->up);
			clean_plot(n);
			unsavedChange = true;		// signal unsaved change
			}
			*choice=0;
			return r;
			break;

//Changes the number of instances of only the Object type shown
//in the browser
case 33:
*choice=0;
cmd(inter, "destroy .l .m");
if(r->up==NULL)
 {
  cmd(inter, "tk_messageBox -type ok -message \"You cannot create many copies of the 'Root' object.\nConsider, if necessary, to add a new parent object here: all the elements will be moved in the newly created object, which can be multiplied in many copies.\"");
  *choice=0;
  return r;
 }
if(actual_steps>0)
 {cmd(inter, "toplevel .warn");
  cmd(inter, "wm title .warn \"Warning\"");
  cmd(inter, "wm transient .warn .");
  cmd(inter, "label .warn.l1 -text \"Warning\" -foreground red");
  cmd(inter, "label .warn.l -text \"Simulation already run\nLoad again the model to make changes to the Object's numbers\"");
  cmd(inter, "button .warn.b -text Ok -command {set choice 1}");
  cmd(inter, "pack .warn.l1 .warn.l .warn.b");
  cmd(inter, "focus -force .warn");
  cmd(inter, "bind .warn <KeyPress-Return> {.warn.b invoke}");
#ifndef DUAL_MONITOR
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
  while(*choice==0)
   Tcl_DoOneEvent(0);
  *choice=0;
  cmd(inter, "destroy .warn");
  return r;
 }

 skip_next_obj(r, &num);
Tcl_LinkVar(inter, "num", (char *) &num, TCL_LINK_INT);
sprintf(msg, "set num %d",num);
cmd(inter, msg);
cmd(inter, "label .l1 -text \"Insert new number of Instances for all groups of Objects\"");
sprintf(msg, "label .l2 -text \"%s\"",r->label);
cmd(inter, msg);
cmd(inter, "label .l3 -text \"Warning: All groups of these objects will be affected\"");
cmd(inter, "entry .ent -width 20 -relief sunken -textvariable num");

cmd(inter, "frame .cp -relief groove -bd 2");
cmd(inter, "label .cp.l -text \"Copy from instance: \"");
cmd(inter, "set cfrom 1");
cmd(inter, "entry .cp.e -textvariable cfrom -width 10");
cmd(inter, "button .cp.compute -text Compute -command {set conf 1; set choice 3}");
cmd(inter, "pack .cp.l .cp.e .cp.compute -side left");

cmd(inter, "button .ok -text Ok -command {set choice 1}");
cmd(inter, "button .can -text Cancel -command {set choice 2}"); 
cmd(inter, "button .hel -text Help -command {LsdHelp mdataobjn.html#this}"); 
cmd(inter, "pack .l1 .l2 .l3 .cp .ent .ok .hel .can");
cmd(inter, "bind .ent <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .ent <KeyPress-Escape> {set choice 2}");
cmd(inter, ".ent selection range 0 end");
cmd(inter, "focus -force .ent");


here_objec_num1:
set_window_size();
while(*choice==0)
 Tcl_DoOneEvent(0);

 if(*choice==3)
  {k=compute_copyfrom(r, choice);
   if(k>0)
   {
   sprintf(msg, "set cfrom %d",k);
   cmd(inter, msg);
   }
   cmd(inter, "set conf 0");
   *choice=0;
   goto here_objec_num1;
  } 

cmd(inter,"destroy .l1 .l2 .l3 .cp .ent .ok .can .hel");
Tcl_UnlinkVar(inter, "num");

*choice=0;
if(*choice==2)
 return r;
cmd(inter, "set choice $cfrom");
k=*choice;
*choice=0;
for(i=0, cur=r->up;cur!=NULL; i++, cur=cur->up); 
chg_obj_num(&r, num, i, NULL, choice,k);

unsavedChange = true;		// signal unsaved change
*choice=0;
return r;
break;


//Browse through the model instance by instance
case 34:
cmd(inter, "destroy .l .m");
*choice=0;
cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
deb(r, NULL,NULL,&fake);
cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
*choice=0;
return r;
break;

//Windows destroyed
case 35:
if ( discard_change( ) )	// check for unsaved configuration changes
myexit(0);
break;

case 36:
//create model report
for(n=r; n->up!=NULL; n=n->up);
cmd(inter, "destroy .l .m");
cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
report(choice, n);
cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
*choice=0;
return r;

//save result
case 37:
*choice=0;
cmd(inter, "destroy .l .m");
if(actual_steps==0)
 {cmd(inter, "toplevel .warn");
  cmd(inter, "wm title .warn \"Warning\"");
  cmd(inter, "wm transient .warn .");
  cmd(inter, "label .warn.l -text \"Simulation not run\"");
  cmd(inter, "button .warn.b -text Ok -command {set choice 1}");
  cmd(inter, "pack .warn.l .warn.b");
  cmd(inter, "focus -force .warn.b");
#ifndef DUAL_MONITOR
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");  
#else
  cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");  
#endif
  while(*choice==0)
   Tcl_DoOneEvent(0);
  *choice=0;
  cmd(inter, "destroy .warn");
  return r;
 }

//Choose a name
  cmd(inter, "toplevel .n");
  cmd(inter, "wm title .n \"Name\"");
  cmd(inter, "wm transient .n .");
  cmd(inter, "label .n.l -text \"Choose a name for result files.\\nAll data saved will be stored in the file '.res'\\nand the configuration that produced will be copied in a new file '.lsd'.\"");
  cmd(inter, "set lab \"Res1\"");
  cmd(inter, "entry .n.e -width 20 -relief sunken -textvariable lab");
  cmd(inter, "set dozip 0");
  cmd(inter, "checkbutton .n.dozip -text \"Generate zipped results file\" -variable dozip");
  cmd(inter, "button .n.b -text Ok -command {set choice 1}");
  cmd(inter, "button .n.c -text Cancel -command {set choice 2}");
  cmd(inter, "focus -force .n.e");
  cmd(inter, "pack .n.l .n.e .n.dozip .n.b .n.c");
  cmd(inter, "bind .n <KeyPress-Return> {set choice 1}");
  cmd(inter, "bind .n <KeyPress-Escape> {set choice 2}");
#ifndef DUAL_MONITOR
  cmd(inter, "set w .n; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");  
#else
  cmd(inter, "set w .n; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");  
#endif
  cmd(inter, ".n.e selection range 0 end");  
  while(*choice==0)
   Tcl_DoOneEvent(0);
  cmd(inter, "bind . <KeyPress-Return> {}");
  cmd(inter, "bind . <KeyPress-Escape> {}");

  cmd(inter, "if {[string length lab] == 0} {set choice 2}");
  cmd(inter, "destroy .n");
  if(*choice==2)
  { *choice=0;
    return r;
  }

lab1=(char *)Tcl_GetVar(inter, "lab",0);
strcpy(lab, lab1);
cmd(inter, "set choice $dozip");
dozip=*choice;
sprintf(msg, "%s.lsd", simul_name);
sprintf(ch, "%s.lsd", lab);
sprintf(msg, "file copy -force %s.lsd %s.lsd", simul_name, lab);
cmd(inter, msg);
sprintf(msg, "\nLsd result file: %s.res%s\nLsd data file: %s.lsd\nSaving data ...",lab, dozip?".gz":"", lab);
plog(msg);
cmd(inter, "focus .log");
//cmd(inter, "raise .log");
cmd(inter, "if { [winfo exists .model_str] == 1} {wm withdraw .model_str} {}");
cmd(inter, "update");

if( strlen( path ) == 0 )
	sprintf( msg, "%s.res", lab );
else
	sprintf( msg, "%s/%s.res", path, lab );
	
rf = new result( msg, "wt", dozip );	// create results file object
for( n = r; n->up != NULL; n = n->up );	// get root object
rf->title( n, 1 );						// write header
rf->data( n, 0, actual_steps );			// write all data
delete rf;								// close file and delete object
plog(" done\n");
*choice=0;
cmd(inter, "wm deiconify .");
//cmd(inter, "if { [winfo exists $c] == 1} {wm deiconify $c} {}");
return r;
break;
/*
//Re-load last model
case 380:
        *choice=0;
        t=0;
		  cmd(inter, "destroy .l .m");
        if(struct_loaded==0)
          break;
        for(n=r; n->up!=NULL; n=n->up);
		  n->empty();
      empty_cemetery();
      empty_descr();
      add_description("Root", "Object", "(no description available)");
		  n->label = new char[strlen("Root")+1];
		  strcpy(n->label, "Root");
		  r=n;
		  cmd(inter, "if {[winfo exists $c.c]==1} {destroy $c.c} {}");
        actual_steps=0;

   	f=fopen(struct_file, "r");
    cmd(inter, "catch {unset ModElem}");
     r->load_struct(f);
	  struct_loaded=1;
	  fclose(f);
	  f=NULL;
	  r->load_param(struct_file, 1,f);
	  show_graph(r);
	 f=search_str(struct_file, "SIM_NUM");
	 if(f!=NULL)
	  fscanf(f, "%d", &sim_num);
	 fclose(f);
	 f=search_str(struct_file, "SEED");
	 if(f!=NULL)
	  fscanf(f, "%d", &seed);
	 fclose(f);
	 f=search_str(struct_file, "MAX_STEP");
	 if(f!=NULL)
	  fscanf(f, "%d", &max_step);
	 fclose(f);
	 break;
*/
case 41:
//help on lsd
cmd(inter, "destroy .l .m");

cmd(inter, "LsdHelp QuickHelp.html");
*choice=0;
break;


case 43:
//Create automatically the documentation
cmd(inter, "destroy .l .m");

*choice=0;

cmd(inter, "wm iconify .");
cmd(inter, "set answer [tk_messageBox -message \"The automatic documentation will replace any previous documentation.\\nDo you want to proceed?\" -type yesno -title Warning -icon warning]");
cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 0} {set choice 1}");


if(*choice==1)
 {
  cmd(inter, "wm deiconify .");
  *choice=0;
  break;
 } 

cmd(inter, "wm iconify .");
cmd(inter, "toplevel .warn");

cmd(inter, "wm transient .warn .");
cmd(inter, "wm title .warn \"Auto Documentation\"");
cmd(inter, "label .warn.l -text \"Choose which elements of the model must replace their documentation.\"");
cmd(inter, "frame .warn.o");

cmd(inter, "radiobutton .warn.o.var -text Variables -variable x -value 1");
cmd(inter, "radiobutton .warn.o.all -text \"All elements\" -variable x -value 2");
cmd(inter, "pack .warn.o.var .warn.o.all -anchor w");
cmd(inter, "frame .warn.f");
cmd(inter, "button .warn.f.ok -text \" Ok \" -command {set choice 1}");
cmd(inter, "button .warn.f.esc -text \" Cancel \" -command {set choice 2}");
cmd(inter, "button .warn.f.hlp -text \" Help \" -command {LsdHelp menumodel.html#auto_docu}");
cmd(inter, "pack .warn.f.ok .warn.f.hlp .warn.f.esc -side left");
cmd(inter, "pack .warn.l .warn.o .warn.f");

cmd(inter, "focus -force .warn.f.ok");
cmd(inter, "bind .warn <KeyPress-Return> {.warn.f.ok invoke}");
cmd(inter, "bind .warn <KeyPress-Escape> {.warn.f.esc invoke}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .warn; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
cmd(inter, "set x 1");
while(*choice==0)
 Tcl_DoOneEvent(0);
cmd(inter, "destroy .warn");
cmd(inter, "wm deiconify ."); 

if(*choice==2)
 {*choice=0;
  break;
 } 
cmd(inter, "set choice $x");
if(*choice==1)
   auto_document( choice, NULL, "VARIABLES");
else
   auto_document( choice, NULL, "ALL"); 

*choice=0;
break;


case 44:
//see model report
cmd(inter, "destroy .l .m");
sprintf(name_rep, "report_%s.html", simul_name);
sprintf(msg, "set choice [file exists %s]", name_rep);
cmd(inter, msg);
if(*choice == 0)
 {
  cmd(inter, "set answer [tk_messageBox -message \"Model report not found.\\nYou may create a model report file from menu Model.\\nDo you want to look for another HTML file?\" -type yesno -title Warning -icon warning]");
  cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 1} {set choice 0}");
 if(*choice == 0)
  break;
 cmd(inter, "set fname [tk_getOpenFile -title \"Load HTML File\" -defaultextension \".html\" -initialdir [pwd] -filetypes {{{HTML Files} {.html}} {{All Files} {*}} }]");
 cmd(inter, "if {$fname == \"\"} {set choice 0} {set fname [file tail $fname]; set choice 1}");
 if(*choice == 0)
  break;

 }
else
 {
 sprintf(msg, "set fname %s", name_rep);
 cmd(inter, msg);
 }

//cmd(inter, "set app $tcl_platform(platform)");
lab1=(char *)Tcl_GetVar(inter, "app",0);
cmd(inter, "set app $tcl_platform(os)");
cmd(inter, "set app $tcl_platform(osVersion)");

lab1=(char *)Tcl_GetVar(inter, "app",0);

if(*choice==1) //model report exists
  cmd(inter, "LsdHtml $fname");
  
*choice=0;
break;

case 45:
//save descriptions
cmd(inter, "destroy .l .m");
lab1=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(lab, lab1);

change_descr_text(lab);

unsavedChange = true;		// signal unsaved change
*choice=0;
break;

case 46:
//show the equation of a variable 
cmd(inter, "destroy .l .m");
lab1=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(lab, lab1);
show_eq(lab, choice);
*choice=0;
break;

case 47:
//show the equations where vname is used 
cmd(inter, "destroy .l .m");
lab1=(char *)Tcl_GetVar(inter, "vname",0);
strcpy(lab, lab1);

scan_used_lab(lab, choice);

*choice=0;
break;
case 48:
//set the Html browser for Unix systems
cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Set Browser\"");

cmd(inter, "set temp_var $HtmlBrowser");
cmd(inter, "label .a.l2 -text \"HTML Browser to use for help pages.\"");
cmd(inter, "entry .a.v_num2 -width 30 -textvariable temp_var");
cmd(inter, "bind .a.v_num2 <Return> {focus -force .a.f.ok}");


cmd(inter, "frame .a.f");	
cmd(inter, "button .a.f.ok -text Ok -command {set choice 1}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");
cmd(inter, "button .a.f.esc -text Cancel -command {set choice 2}");
cmd(inter, "bind .a <Escape> {.a.f.esc invoke}");
cmd(inter, "button .a.f.help -text Help -command {LsdHelp lsdfuncMacro.html#V}");
cmd(inter, "button .a.f.def -text Default -command {set temp_var mozilla}");
cmd(inter, "bind .a.f.ok <Return> {.a.f.ok invoke}");

cmd(inter, "pack .a.f.ok .a.f.help .a.f.def .a.f.esc -side left");
cmd(inter, "pack .a.l2 .a.v_num2 .a.f");

//cmd(inter, "set w .a; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y");
#ifndef DUAL_MONITOR
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .a; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif

cmd(inter, "focus -force .a.v_num2");
cmd(inter, ".a.v_num2 selection range 0 end");

*choice=0;
while(*choice==0)
 Tcl_DoOneEvent(0);

if(*choice==1)
 cmd(inter, "set HtmlBrowser $temp_var");

cmd(inter, "destroy .a");
cmd(inter, "destroy .l .m");
*choice=0;
break;


case 50: //find an element of the model
cmd(inter, "destroy .l .m");
cmd(inter, "toplevel .s");
cmd(inter, "wm transient .s .");
cmd(inter, "wm title .s \"Find element\"");
cmd(inter, "frame .s.i -relief groove -bd 2");
cmd(inter, "label .s.i.l -text \"Type the initial letters of the variable or parameter. The system will propose a name.\nPress Enter when the desired label appears.\"");
cmd(inter, "set bidi \"\"");
cmd(inter, "entry .s.i.e -textvariable bidi");
cmd(inter, "pack .s.i.l .s.i.e");
cmd(inter, "button .s.ok -text Ok -command {set choice 1}");
cmd(inter, "button .s.esc -text Cancel -command {set choice 2}");

cmd(inter, "pack .s.i .s.ok .s.esc");
cmd(inter, "bind .s.i.e <KeyPress-Return> {set choice 1}");
cmd(inter, "bind .s <KeyPress-Escape> {set choice 2}");
#ifndef DUAL_MONITOR
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 - [winfo vrootx [winfo parent $w]]]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 - [winfo vrooty [winfo parent $w]]]; wm geom $w +$x+$y; update; wm deiconify $w");
#else
cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
#endif
*choice=0;
cmd(inter, "focus .s.i.e");
cmd(inter, "bind .s.i.e <KeyRelease> {if { %N < 256 } { set b [.s.i.e index insert]; set c [.s.i.e get]; set f [lsearch -glob $ModElem $c*]; if { $f !=-1 } {set d [lindex $ModElem $f]; .s.i.e delete 0 end; .s.i.e insert 0 $d; .s.i.e index $b; .s.i.e selection range $b end } { } } { } }");
  while(*choice==0)
	Tcl_DoOneEvent(0);
cmd(inter, "destroy .s");
if(*choice==2)
 {*choice=0;
  break;
 }

case 55: //arrive here from the list of vars used.
if(*choice==55)
 cmd(inter, "destroy .l .m");
*choice=0;
lab1=(char *)Tcl_GetVar(inter, "bidi",0);
strcpy(msg,lab1);
no_error=1;
cv=r->search_var(r, msg);
no_error=0;
if(cv!=NULL)
 {
 for(i=0, cur_v=cv->up->v; cur_v!=cv; cur_v=cur_v->next, i++);
 sprintf(msg, "set cur %d; set listfocus 1; set itemfocus $cur", i);
 cmd(inter, msg);
 return cv->up;
 }
break;

case 5600: //choose lattice type of updating

*choice=0;
cmd(inter, "destroy .l .m");

Tcl_LinkVar(inter, "lattype", (char *) &done, TCL_LINK_INT);
done=lattice_type;
cmd(inter, "set a $lattype");
cmd(inter, "toplevel .a");
cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"Set Lattice Updating\"");

cmd(inter, "label .a.l -text \"Type of lattice updating\" -fg red");
cmd(inter, "pack .a.l");
cmd(inter, "frame .a.v");
cmd(inter, "radiobutton .a.v.r1 -text \"Lattice updating type 1. More efficient when the a cell changes many times\" -variable lattype -value 1\"");
cmd(inter, "radiobutton .a.v.r2 -text \"Lattice updating type 2. More efficient when cells change rarely\" -variable lattype -value 2\"");

cmd(inter, "pack .a.v.r1 .a.v.r2 -anchor w");

cmd(inter, "pack .a.v -side left -fill y -fill x");


cmd(inter, "button .a.ok -text \" Ok \" -command {set choice 1}");
cmd(inter, "pack .a.ok -side bottom");

while(*choice==0)
 Tcl_DoOneEvent(0);

lattice_type=done;
*choice=0;
Tcl_UnlinkVar(inter, "lattype");
cmd(inter, "destroy .a");
break;
case 51: //upload in memory current equation file
/*
replace lsd_eq_file with the eq_file. That is, make appear actually used equation file as the one used for the current configuration
*/
cmd(inter, "destroy .l .m");
if(!strcmp(eq_file, lsd_eq_file))
 {//no need to do anything
  *choice=0;
  break;
 }
cmd(inter, "set answer [tk_messageBox -title Warning -icon warning -message \"The equations associated to the configuration file are going to be replaced with the equations used for the Lsd model program.\\nPress Ok to confirm.\" -type okcancel ]");
cmd(inter, "if {[string compare $answer \"yes\"] == 0} {set choice 1} {set choice 0}");
 if(*choice == 0)
  break;
strcpy(lsd_eq_file, eq_file);
unsavedChange = true;		// signal unsaved change
*choice=0;
break;

case 52: //offload configuration's equations in a new equation file
/*
Used to re-generate the equations used for the current configuration file
*/
if(strlen(lsd_eq_file)==0 || !strcmp(eq_file, lsd_eq_file) )
 {cmd(inter, "tk_messageBox -title Warning -icon warning -message \"There are no equations to be offloaded differing from the current equation file.\" -type ok");
 cmd(inter, "destroy .l .m");
 *choice=0;
 break;
 }
cmd(inter, "destroy .l .m");

sprintf(msg, "set res fun_%s.cpp", simul_name);
cmd(inter, msg);
sprintf(msg, "set tk_strictMotif 0; set bah [tk_getSaveFile -title \"Save Equation File\" -defaultextension \".cpp\" -initialfile $res -initialdir [pwd] -filetypes {{{Lsd Equation Files} {.cpp}} {{All Files} {*}} }]; set tk_strictMotif 1");
cmd(inter, msg);

cmd(inter,"if {[string length $bah] > 0} { set choice 1; set res1 [file tail $res]} {set choice 0}");
if ( *choice == 0 || ! discard_change( ) )	// esc or unsaved configuration?
  break;

lab1=(char *)Tcl_GetVar(inter, "res",0);
strcpy(lab, lab1);

if(strlen(lab)==0)
 break;
f=fopen(lab, "wt");  // use text mode for Windows better compatibility
fprintf(f, "%s", lsd_eq_file);
fclose(f);
cmd(inter, "tk_messageBox -title Warning -icon warning -message \"The new equation file\\n$res1\\nhas been created.\\nYou need to generate a new Lsd model program to use these equations, replacing the name of the equation file in LMM with the command 'Model Compilation Options' (menu Model).\" -type ok");
*choice=0;
break;


case 53: //Compare equation files
/*

*/
cmd(inter, "destroy .l .m");
if ( strlen( lsd_eq_file ) == 0 || ! discard_change( ) )	// esc or unsaved configuration?)
 {*choice=0;
  break;
 } 
f=fopen("temp.tmp", "wt");  // use text mode for Windows better compatibility
fprintf(f, "%s", lsd_eq_file);
fclose(f);

read_eq_filename(lab);
sprintf(msg, "LsdTkDiff %s temp.tmp", lab);
cmd(inter, msg);

*choice=0;
break;

case 54:
 //toggle ignore eq. file controls
cmd(inter, "destroy .l .m");
cmd(inter, "set choice $ignore_eq_file");
ignore_eq_file=*choice;
*choice=0;
break; 

case 56:
 //toggle fast lattice updating
cmd(inter, "destroy .l .m");
cmd(inter, "set choice $lattype");
lattice_type=*choice;
*choice=0;
break; 

case 57:
cmd(inter, "destroy .l .m");

f=fopen("tex_report.tex", "wt");
fprintf(f,"\\newcommand{\\lsd}[1] {\\textit{#1}}\n");
tex_report_init(root,f);
tex_report_observe(root, f);
tex_report(root,f);

fclose(f);
*choice=0;
break;

case 58:
cmd(inter, "destroy .m .l");

lab1=(char *)Tcl_GetVar(inter, "vname",0);
sscanf(lab1, "%s", lab_old);
sprintf(msg, "set vname %s",lab_old);
cmd(inter, msg);
//show_description(lab_old);
shift_var(-1, lab_old, r);
unsavedChange = true;		// signal unsaved change
*choice=0;
return r;

case 59:
cmd(inter, "destroy .m .l");

lab1=(char *)Tcl_GetVar(inter, "vname",0);
sscanf(lab1, "%s", lab_old);
sprintf(msg, "set vname %s",lab_old);
cmd(inter, msg);
//show_description(lab_old);
shift_var(1, lab_old, r);
unsavedChange = true;		// signal unsaved change
*choice=0;
return r;

case 60:
cmd(inter, "destroy .m .l");

lab1=(char *)Tcl_GetVar(inter, "vname",0);
sscanf(lab1, "%s", lab_old);
sprintf(msg, "set vname %s",lab_old);
cmd(inter, msg);
//show_description(lab_old);
shift_desc(-1, lab_old, r);
unsavedChange = true;		// signal unsaved change
*choice=0;
return r;

case 61:
cmd(inter, "destroy .m .l");

lab1=(char *)Tcl_GetVar(inter, "vname",0);
sscanf(lab1, "%s", lab_old);
sprintf(msg, "set vname %s",lab_old);
cmd(inter, msg);
//show_description(lab_old);
shift_desc(1, lab_old, r);
unsavedChange = true;		// signal unsaved change
*choice=0;
return r;

//Create parallel sensitivity analysis configuration
case 62:
cmd(inter, "destroy .m .l");

if (rsense!=NULL) 
  {
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);
	long ptsSa = num_sensitivity_points(rsense);	// total number of points in sensitivity space
	sprintf(msg, "\nSensitivity analysis space size: %ld", ptsSa);
	plog(msg);
	// Prevent running into too big sensitivity spaces (high computation times)
	if(ptsSa > MAX_SENS_POINTS)
	{
		plog("\nWarning: sensitivity analysis space size is too big!");
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
    for (i=1,cs=rsense; cs!=NULL; cs=cs->next)
        i*=cs->nvalues;
    cur=root->b->head;
    root->add_n_objects2(cur->label, i-1, cur);
    sensitivity_parallel(cur,rsense);
	unsavedChange = true;		// signal unsaved change
 	cmd(inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has changed your model structure, replicating the entire model for each sensitivity configuration.\\n\\nIf you want to preserve your original configuration file, save your new configuration using a different name BEFORE running the model.\"");
  }
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Init. Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");
	
*choice=0;
return r;

//Create sequential sensitivity analysis configuration
case 63:
cmd(inter, "destroy .m .l");

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);
	long ptsSa = num_sensitivity_points(rsense);	// total number of points in sensitivity space
	sprintf(msg, "\nSensitivity analysis space size: %ld", ptsSa);
	plog(msg);
	// Prevent running into too big sensitivity spaces (high computation times)
	if(ptsSa > MAX_SENS_POINTS)
	{
		plog("\nWarning: sensitivity analysis space size is too big!");
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
    findexSens=1;
    sensitivity_sequential(&findexSens,rsense);
	sprintf( msg, "\nSensitivity analysis configurations produced: %ld", findexSens - 1 );
	plog( msg );
 	cmd(inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the sequential sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"");
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Init. Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

*choice=0;
return r;

//Create Monte Carlo (MC) random sensitivity analysis sampling configuration
case 71:
cmd(inter, "destroy .m .l");

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);
	long maxMC = num_sensitivity_points(rsense);	// total number of points in sensitivity space
	sprintf(msg, "\nSensitivity analysis space size: %ld", maxMC);
	plog(msg);

	// get the number of Monte Carlo samples to produce
	double sizMC;
	Tcl_LinkVar(inter, "sizMC", (char *)&sizMC, TCL_LINK_DOUBLE);
	cmd(inter, "toplevel .s");
	cmd(inter, "wm transient .s .");
	cmd(inter, "wm title .s \"Num. of MC samples\"");
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "label .s.i.l -text \"Type the Monte Carlo sample size\nas % of sensitivity space size.\n(from 0 to 100)\"");
	cmd(inter, "set sizMC \"10.0\"");
	cmd(inter, "entry .s.i.e -justify center -font {-weight bold} -textvariable sizMC");
	cmd(inter, ".s.i.e selection range 0 end");
	cmd(inter, "label .s.i.w -text \"Requesting a too big\nsample is not recommended.\nThe sample size represents the\napproximated target average.\"");
	cmd(inter, "pack .s.i.l .s.i.e .s.i.w");
	cmd(inter, "button .s.ok -text Ok -command {set choice 1}");
	cmd(inter, "button .s.esc -text Cancel -command {set choice 0}");
	cmd(inter, "pack .s.i .s.ok .s.esc -fill x");
	cmd(inter, "bind .s <KeyPress-Return> {set choice 1}");
	cmd(inter, "bind .s <KeyPress-Escape> {set choice 0}");
	cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
	*choice = -1;
	cmd(inter, "focus .s.i.e");
	while(*choice == -1)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroy .s");
	Tcl_UnlinkVar(inter, "sizMC");
	if(*choice == 0)
		break;
	
	// Check if number is valid
	sizMC /= 100.0;
	if( (sizMC * maxMC) < 1 || sizMC > 1.0)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid Monte Carlo sample size to perform the sensitivity analysis.\\n\\nSelect a number between 0% and 100% that produces at least one sample (in average).\"");
		*choice=0;
		break;
	}

	// Prevent running into too big sensitivity space samples (high computation times)
	if((sizMC * maxMC) > MAX_SENS_POINTS)
	{
		sprintf(msg, "\nWarning: sampled sensitivity analysis space size (%ld) is still too big!", (long)(sizMC * maxMC));
		plog(msg);
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}");
		if(*choice == 0)
			break;
	}
	
	sprintf(msg, "\nTarget sensitivity analysis sample size: %ld (%.1f%%)", (long)(sizMC * maxMC), 100 * sizMC);
	plog(msg);
    findexSens=1;
    sensitivity_sequential(&findexSens, rsense, sizMC);
	sprintf(msg, "\nSensitivity analysis samples produced: %ld", findexSens - 1);
	plog(msg);
 	cmd(inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the Monte Carlo sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"");
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Init. Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

*choice=0;
return r;

//Create Near Orthogonal Latin Hypercube (NOLH) sensitivity analysis sampling configuration
case 72:
cmd(inter, "destroy .m .l");

if (rsense!=NULL) 
{
	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	int varSA = num_sensitivity_variables(rsense);	// number of variables to test
	sprintf(msg, "\nNumber of variables for sensitivity analysis: %d", varSA);
	plog(msg);
	long ptsSa = num_sensitivity_points(rsense);	// total number of points in sensitivity space
	sprintf(msg, "\nSensitivity analysis space size: %ld", ptsSa);
	plog(msg);

	cmd(inter, "toplevel .s");
	cmd(inter, "wm transient .s .");
	cmd(inter, "wm title .s \"NOLH DoE\"");
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "label .s.i.l -text \"Type the name of the design\nfile to be used. Or press\n'Built-in' to proceed with\ninternal tables (up to 29 factors).\"");
	cmd(inter, "set NOLHfile \"NOLH.csv\"");
	cmd(inter, "entry .s.i.e -justify center -font {-weight bold} -textvariable NOLHfile");
	cmd(inter, ".s.i.e selection range 0 end");
	cmd(inter, "label .s.i.w -text \"The file must be located\nin the same folder of the\nselected configuration file.\nThe file must be in CSV\nformat with NO empty lines.\"");
	cmd(inter, "pack .s.i.l .s.i.e .s.i.w");
	cmd(inter, "button .s.ok -text \"From File\" -command {set choice 1}");
	cmd(inter, "button .s.int -text Built-in -command {set choice 2}");
	cmd(inter, "button .s.esc -text Cancel -command {set choice 0}");
	cmd(inter, "pack .s.i .s.ok .s.int .s.esc -fill x");
	cmd(inter, "bind .s <KeyPress-Escape> {set choice 0}");
	cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
	*choice = -1;
	cmd(inter, "focus .s.i.e");
	while(*choice == -1)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroy .s");
	if(*choice == 0)
		break;
	
	char NOLHfile[300];
	if ( *choice == 2 )
		strcpy( NOLHfile, "" );
	else
	{
		char const *fname = Tcl_GetVar(inter, "NOLHfile", 0);
		strcpy(NOLHfile, fname);
	}

	// adjust an NOLH design of experiment (DoE) for the sensitivity data
	design *NOLHdoe = new design( rsense, 1, NOLHfile );
	
	if ( NOLHdoe -> n == 0 )					// DoE configuration is not ok?
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"It was not possible to create a Non Orthogonal Latin Hypercube (NOLH) Design of Experiment (DoE) for the current sensitivity configuration.\\n\\nIf the number of variables (factors) is large than 29, an external NOLH has to be provided in the file NOLH.csv (empty lines not allowed).\"" );
		goto end72;
	}

	// Prevent running into too big sensitivity space samples (high computation times)
	if ( NOLHdoe -> n > MAX_SENS_POINTS )
	{
		sprintf( msg, "\nWarning: sampled sensitivity analysis space size (%d) is still too big!", NOLHdoe -> n );
		plog( msg );
		cmd( inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"Too many cases to perform the sensitivity analysis!\n\nPress 'Ok' if you want to continue anyway or 'Cancel' to abort the command now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 0}}" );
		if( *choice == 0 )
			goto end72;
	}
	
    findexSens = 1;
    sensitivity_doe( &findexSens, NOLHdoe );
	sprintf( msg, "\nSensitivity analysis samples produced: %ld", findexSens - 1 );
	plog( msg );
 	cmd( inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"Lsd has created configuration files for the Monte Carlo sensitivity analysis.\\n\\nTo run the analysis first you have to create a 'No Window' version of the model program, using the 'Model'/'Generate 'No Window' Version' option in LMM and following the instructions provided. This step has to be done every time you modify your equations file.\\n\\nThen execute this command in the directory of the model:\\n\\n> lsd_gnuNW  -f  <configuration_file>  -s  <n>\\n\\nReplace <configuration_file> with the name of your original configuration file WITHOUT the '.lsd' extension and <n> with the number of the first configuration file to run (usually 1).\"" );

end72:
	delete NOLHdoe;
}
else
 	cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Before using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Init. Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");

*choice=0;
return r;

//Load a sensitivity analysis configuration
case 64:
	cmd(inter, "destroy .l .m");
	
	// check a model is already loaded
	if(struct_loaded==0)
	{ 
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"There is no model loaded.\\n\\nPlease load one before trying to load a sensitivity analysis configuration.\"");
		break;
    } 
	// check for existing sensitivity data loaded
	if (rsense!=NULL) 
	{
		cmd(inter, "set answer [tk_messageBox -type okcancel -icon warning -default cancel -title \"Sensitivity Analysis\" -message \"There is sensitivity data already loaded.\\n\\nPress 'Ok' if you want to discard the existing data before loading a new sensitivity configuration.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 2}}");
		if(*choice == 2)
			break;
		
		// empty sensitivity data
		empty_sensitivity(rsense); 			// discard read data
		rsense=NULL;
		unsavedSense = false;				// nothing to save
		findexSens=0;
	}
	// set default name and path to conf. file folder
	sprintf(lab, "set res %s_sens", simul_name);
	cmd(inter, lab);
	if(strlen(path)>0)
		sprintf(msg, "set path \"%s\"", path);
	else
		sprintf(msg, "set path [pwd]");
	cmd(inter, msg);
	cmd(inter, "cd $path");
	// open dialog box to get file name & folder
	sprintf(msg, " set bah [tk_getOpenFile -title \"Load Sensitivity Analysis Data\" -defaultextension \".txt\" -initialfile $res -initialdir $path  -filetypes {{{Sensitivity analysis text files} {.txt}}  }]");
	cmd(inter, msg);
	cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .txt $res] -1];set res [string range $res 0 $last]} {set choice 2}");
	if(*choice==2)
		break;
	lab1=(char *)Tcl_GetVar(inter, "res",0);
	lab2=(char *)Tcl_GetVar(inter, "path",0);
	// form full name
	if(sens_file!=NULL)
		delete sens_file;
	sens_file=new char[strlen(lab1)+strlen(lab2)+7];
	if(strlen(lab1)>0)
	{
		cmd(inter, "cd $path");
		sprintf(sens_file,"%s/%s.txt",lab2,lab1);
	}
	else
		sprintf(sens_file,"%s.txt",lab1);
	// read sensitivity file (text mode)
	f=fopen(sens_file, "rt");
	if(f==NULL)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Sensitivity analysis file not found.\"");
		break;
	}

	// read data from file (1 line per element, '#' indicate comment)
	while(!feof(f))
	{	// read element by element, skipping comments
		fscanf(f, "%s", lab);				// read string
		while(lab[0]=='#')					// start of a comment
		{
			do								// jump to next line
				cc=fgetc(f);
			while(!feof(f) && cc!='\n');
			fscanf(f, "%s", lab);			// try again
		}

		if(feof(f))							// ended too early?
			break;

		for(n=r; n->up!=NULL; n=n->up);		// check if element exists
		cv=n->search_var(n,lab);
		if(cv==NULL || (cv->param!=1 && cv->num_lag==0))
			goto error64;					// and not parameter or lagged variable
		// create memory allocation for new variable		
		if (rsense==NULL)					// allocate first element
			rsense=cs=new sense;
		else								// allocate next ones
		{
			cs->next=new sense;
			cs=cs->next;
		}
		cs->v=NULL;							// initialize struct pointers
		cs->next=NULL;

		cs->label=new char[strlen(lab)+1];  // save element name
		strcpy(cs->label,lab);
		// get lags and # of values to test
		if(!fscanf(f, "%d %d:", &cs->lag, &cs->nvalues))
			goto error64;
		
		if(cs->lag==0)						// adjust type and lag #
			cs->param=1;
		else
		{
			cs->param=0;
			cs->lag=abs(cs->lag)-1;
		}

		cs->v=new double[cs->nvalues];		// get values
		for (i=0; i < cs->nvalues; i++)
			if(!fscanf(f, "%lf", &cs->v[i]))
				goto error64;
	}	
	fclose(f);
	break;
	
	// error handling
	error64:
		empty_sensitivity(rsense); 			// discard read data
		rsense=NULL;
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid sensitivity analysis file.\"");
		fclose(f);
		break;

//Save a sensitivity analysis configuration
case 65:
	cmd(inter, "destroy .l .m");

	// check for existing sensitivity data loaded
	if (rsense==NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon warning -title \"Sensitivity Analysis\" -message \"There is no sensitivity data to save.\\n\\nBefore using this option you have to select at least one parameter or lagged variable to perform the sensitivity analysis and inform their values.\\n\\nTo set the sensitivity analysis ranges of values, use the 'Data'/'Init. Values' menu option, click on 'Set All' in the appropriate parameters and variables, select 'Sensitivity Analysis' as the initialization function and inform the 'Number of values' to be entered for that parameter or variable.\\nAfter clicking 'Ok', enter the informed number of values, separated by spaces, tabs, commas, semicolons etc. (the decimal point has to be '.'). It's possible to simply paste the list of values from the clipboard.\"");
		break;
	}
	// default file name and path
	sprintf(lab, "set res %s_sens", simul_name);
	cmd(inter, lab);
	if(strlen(path)>0)
		sprintf(msg, "set path \"%s\"", path);
	else
		sprintf(msg, "set path [pwd]");
	cmd(inter, msg);
	cmd(inter, "cd $path");
	// open dialog box to get file name & folder
	sprintf(msg, " set bah [tk_getSaveFile -title \"Save Sensitivity Analysis Data\" -defaultextension \".txt\" -initialfile $res -initialdir $path  -filetypes {{{Sensitivity analysis text files} {.txt}} {{All Files} {*}} }]");
	cmd(inter, msg);
	cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res];set last [expr [string last .txt $res] -1];set res [string range $res 0 $last]} {set choice 2}");
	if(*choice==2)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Invalid sensitivity analysis file name or path.\"");
		break;
	}
	lab1=(char *)Tcl_GetVar(inter, "res",0);
	lab2=(char *)Tcl_GetVar(inter, "path",0);
	// form full name
	if(sens_file!=NULL)
		delete sens_file;
	sens_file=new char[strlen(lab2)+strlen(lab1)+7];
	if(strlen(lab2)>0)
	{
		cmd(inter, "cd $path");
		sprintf(sens_file,"%s/%s.txt",lab2,lab1);
	}
	else
		sprintf(sens_file,"%s.txt",lab1);
	// write sensitivity file (text mode)
	f=fopen(sens_file, "wt");  // use text mode for Windows better compatibility
	if(f==NULL)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"Sensitivity analysis file not saved.\\n\\nPlease check the file name and path are valid.\"");
		break;
	}
	for(cs=rsense; cs!=NULL; cs=cs->next)
	{
		if(cs->param==1)
			fprintf(f, "%s 0 %d:", cs->label, cs->nvalues);	
		else
			fprintf(f, "%s -%d %d:", cs->label, cs->lag+1, cs->nvalues);
		for(i=0; i<cs->nvalues; i++)
			fprintf(f," %g", cs->v[i]);
		fprintf(f,"\n");
	}
	fclose(f);
	unsavedSense = false;			// nothing to save
	break;

//Show sensitivity analysis configuration
case 66:
	cmd(inter, "destroy .l .m");
	*choice=50;

	// check for existing sensitivity data loaded
	if (rsense==NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon warning -title \"Sensitivity Analysis\" -message \"There is no sensitivity data to show.\"");
		break;
	}
	// print data to log window
	plog("\n\nVariables and parameters set for sensitivity analysis :\n");
	for(cs=rsense; cs!=NULL; cs=cs->next)
	{
		if(cs->param==1)
			sprintf(msg, "Param: %s\t#%d:\t", cs->label, cs->nvalues);
		else
			sprintf(msg, "Var: %s(-%d)\t#%d:\t", cs->label, cs->lag+1, cs->nvalues);
		plog(msg);
		cmd(inter, "");

		for(i=0; i<cs->nvalues; i++)
		{
			sprintf(msg, ".log.text.text.internal insert end \"%g\\t\" highlight", cs->v[i]);
			cmd(inter, msg);
		}
		plog("\n");
	}
    cmd(inter, "if { [winfo exists $c] == 1} {wm withdraw $c} {}");
	break;

//Remove sensitivity analysis configuration
case 67:
	cmd(inter, "destroy .l .m");
	*choice=0;

	// check for existing sensitivity data loaded
	if (rsense==NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon warning -title \"Sensitivity Analysis\" -message \"There is no sensitivity data to remove.\"");
		break;
	}
	
	if ( ! discard_sense( ) )	// unsaved configuration?
		break;

	// empty sensitivity data
	empty_sensitivity(rsense); 			// discard read data
	rsense=NULL;
	unsavedSense = false;				// nothing to save
	findexSens=0;
	break;
	
//Create batch for serial sensitivity analysis job and optionally run it
case 68:
	cmd(inter, "destroy .l .m");

	// check a model is already loaded
	if(struct_loaded==0)
	{ 
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"There is no model loaded.\\n\\nPlease select one before trying to create a script/batch.\"");
		break;
	}

	if ( ! discard_change( false ) )	// unsaved configuration?
		break;

	// check for existing NW executable
	sprintf(ch, "%s/%s", exec_path, exec_file);			// form full executable name
	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
	if(*choice == 1)
	{
		// remove Windows extension, if present
		if((lab1 = strstr(ch, ".exe")) != NULL)
			lab1[0]='\0';
		else
			if((lab1 = strstr(ch, ".EXE")) != NULL) 
				lab1[0]='\0';
			
		strcat(ch, "NW.exe");							// add Windows ending
	}
	else
		strcat(ch, "NW");								// add Unix ending

	if ((f=fopen(ch, "rb")) == NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Sensitivity Analysis\" -message \"The executable file 'lsd_gnuNW' was not found.\n\nPlease create the required executable file using the option 'Model'/'Generate 'No Window' Version' in LMM first.\"");
		break;
	}
	fclose(f);
	
	// check if serial sensitivity configuration was just created
	*choice=0;
	if(findexSens > 0)
		cmd(inter, "set answer [tk_messageBox -type yesnocancel -icon question -default yes -title \"Sensitivity Analysis\" -message \"A sequential sensitivity set of configuration files was just created and will be used to create the script/batch.\n\nPress 'Yes' to confirm or 'No' to select a different set of files.\"]; switch -- $answer {yes {set choice 1} no {set choice 0} cancel {set choice 2}}"); 
	if(*choice == 2)
		break;
	
	// get configuration files to use
	int ffirst, fnext;
	if(*choice == 1)					// use current configuration files
	{
		ffirst=1;
		fnext=findexSens;
		lab1=simul_name;
		lab2=path;
		Tcl_SetVar(inter, "path", path, 0);
	}
	else								// ask for first configuration file
	{
		sprintf(msg, "set res %s_1.lsd", simul_name);	// default name
		cmd(inter, msg);
		if(strlen(path) > 0)							// default path
			sprintf(msg, "set path \"%s\"", path);
		else
			sprintf(msg, "set path [pwd]");
		cmd(inter, msg);
		// open dialog box to get file name & folder
		sprintf(msg, " set bah [tk_getOpenFile -title \"Load first sensitivity analysis configuration file\" -defaultextension \".lsd\" -initialfile $res -initialdir $path  -filetypes {{{Lsd Model Files} {.lsd}}} -multiple no]");
		cmd(inter, msg);
		cmd(inter,"if {[string length $bah] > 0} {set res $bah; set path [file dirname $res]; set res [file tail $res]; set last [expr [string last .lsd $res] - 1]; set res [string range $res 0 $last]; set numpos [expr [string last _ $res] + 1]; set choice [string range $res $numpos end]; set res [string range $res 0 [expr $numpos - 2]]} {set choice 0}");
		if(*choice == 0)
			break;
		ffirst=*choice;
		lab1=(char *)Tcl_GetVar(inter, "res",0);
		lab2=(char *)Tcl_GetVar(inter, "path",0);
		f=NULL;
		do									// search for all sequential files
		{
			if(strlen(lab2) == 0)			// default path
				sprintf(lab, "%s_%d.lsd", lab1, (*choice)++);
			else
				sprintf(lab, "%s/%s_%d.lsd", lab2, lab1, (*choice)++);
			if(f != NULL) fclose(f);
			f=fopen(lab, "r");
		}
		while(f != NULL);
		fnext=*choice - 1;
	}

	// confirm number of cores to use
	cmd(inter, "toplevel .s");
	cmd(inter, "wm transient .s .");
	cmd(inter, "wm title .s \"Num. of CPU cores\"");
	cmd(inter, "frame .s.i -relief groove -bd 2");
	cmd(inter, "label .s.i.l -text \"Type the number of parallel processes to use.\"");
	cmd(inter, "set cores \"4\"");
	cmd(inter, "entry .s.i.e -justify center -font {-weight bold} -textvariable cores");
	cmd(inter, ".s.i.e selection range 0 end");
	cmd(inter, "label .s.i.w -text \"Using a number higher than the number\nof processors/cores is not recommended.\"");
	cmd(inter, "set dozip 0");
	cmd(inter, "checkbutton .s.i.dozip -text \"Generate zipped files\" -variable dozip");
	cmd(inter, "pack .s.i.l .s.i.e .s.i.w .s.i.dozip");
	cmd(inter, "button .s.ok -text Ok -command {set choice $cores}");
	cmd(inter, "button .s.esc -text Cancel -command {set choice 0}");
	cmd(inter, "pack .s.i .s.ok .s.esc -fill x");
	cmd(inter, "bind .s <KeyPress-Return> {set choice $cores}");
	cmd(inter, "bind .s <KeyPress-Escape> {set choice 0}");
	cmd(inter, "set w .s; wm withdraw $w; update idletasks; set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2]; set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2]; wm geom $w +$x+$y; update; wm deiconify $w");
	*choice=-1;
	cmd(inter, "focus .s.i.e");
	while(*choice==-1)
		Tcl_DoOneEvent(0);
	cmd(inter, "destroy .s");
	if(*choice==0)
		break;
	param=*choice;
	cmd(inter, "set choice $dozip");
	dozip=*choice;
	*choice=param;
	
	if(param < 1 || param > 12) param=4;
	
	// create batch file
	char wpath[300];
	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
	if(*choice == 1)
		sprintf(lab, "%s/%s_%d_%d.bat", lab2, lab1, ffirst, fnext - 1);
	else
		sprintf(lab, "%s/%s_%d_%d.sh", lab2, lab1, ffirst, fnext - 1);
	f=fopen(lab, "wt");
	if(*choice == 1)						// Windows header
	{
		fprintf(f, "@echo off\nrem Sequential sensitivity batch generated by Lsd\n");
		fprintf(f, "echo Processing %d configuration files in up to %d parallel processes...\n", fnext - ffirst, param);

		// convert to Windows folder separators (\)
		for(i=0; i < strlen(ch); i++) 
			if(ch[i] == '/') ch[i]='\\';
		strcpy(wpath, lab2);
		for(i=0; i < strlen(wpath); i++) 
			if(wpath[i] == '/') wpath[i]='\\';
	}
	else									// Unix header
	{
		fprintf(f, "#!/bin/bash\n# Sequential sensitivity script generated by Lsd\n");
		fprintf(f, "echo \"Processing %d configuration files in up to %d parallel processes...\"\n", fnext - ffirst, param);
	}
	
	if((fnext - ffirst) > param)			// if possible, work in blocks
	{
		num=(fnext - ffirst)/param;			// base number of cases per core
		sl=(fnext - ffirst)%param;			// remaining cases per core
		for(i=ffirst, j=1; j <= param; j++)	// allocates files by the number of cores
		{
			if(*choice == 1)				// Windows
				fprintf(f, "start \"Lsd Process %d\" /B \"%s\" -f %s\\%s -s %d -e %d %s 1>%s\\%s_%d.log 2>&1\n", j, ch, wpath, lab1, i, j <= sl ? i + num : i + num - 1, dozip ? "-z" : "", wpath, lab1, j);
			else							// Unix
				fprintf(f, "%s -f %s/%s -s %d -e %d %s >%s/%s_%d.log 2>&1 &\n", ch, lab2, lab1, i, j <= sl ? i + num : i + num - 1, dozip ? "-z" : "", lab2, lab1, j);
			 j <= sl ? i+=num+1 : i+=num;
		}
	}
	else									// if not, do one by one
		for(i=ffirst, j=1; i < fnext; i++, j++)
			if(*choice == 1)				// Windows
				fprintf(f, "start \"Lsd Process %d\" /B \"%s\" -f %s\\%s_%d.lsd %s 1>%s\\%s_%d.log 2>&1\n", j, ch, wpath, lab1, i, dozip ? "-z" : "", wpath, lab1, i);
			else							// Unix
				fprintf(f, "%s -f %s/%s_%d.lsd %s >%s/%s_%d.log 2>&1 &\n", ch, lab2, lab1, i, dozip ? "-z" : "", lab2, lab1, i);
	
	if(*choice == 1)						// Windows closing
	{
		fprintf(f, "echo %d log files being generated: %s_1.log to %s_%d.log .\n", j - 1, lab1, lab1, j - 1);
		fclose(f);
	}
	else									// Unix closing
	{
		fprintf(f, "echo \"%d log files being generated: %s_1.log to %s_%d.log .\"\n", j - 1, lab1, lab1, j - 1);
		fprintf(f, "echo \"This terminal shell must not be closed during processing.\"\n");
		fclose(f);
		chmod(lab, ACCESSPERMS);			// set executable perms
	}

	// ask if script/batch should be executed right away
	cmd(inter, "set answer [tk_messageBox -type okcancel -icon question -default cancel -title \"Sensitivity Analysis\" -message \"The script/batch for running the sensitivity configuration files was saved.\n\nPress 'Ok' if you want to start the script/batch as separated processes right now.\"]; switch -- $answer {ok {set choice 1} cancel {set choice 2}}"); 
	if(*choice == 2)
		break;

	// start the job
	sprintf(msg, "set path \"%s\"", lab2);
	cmd(inter, msg);
	cmd(inter, "cd $path");

	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
	if(*choice == 1)						// Windows?
		sprintf(msg, "exec %s &", lab);
	else									// Unix
		sprintf(msg, "exec %s &", lab);
    cmd(inter, msg);

	cmd(inter, "tk_messageBox -type ok -icon info -title \"Sensitivity Analysis\" -message \"The sensitivity analysis script/batch was started in separated processes.\\n\\nThe results and log files are being created in the folder:\\n\\n$path\\n\\nCheck the '.log' files to see the results or use the command 'tail  -F  <name>.log' in a shell/command prompt to follow simulation execution (there is one log file per assigned process/core).\"");
break;

//Start NO WINDOW job as a separate background process
case 69:
	cmd(inter, "destroy .l .m");

	// check a model is already loaded
	if(struct_loaded==0)
	{ 
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Start 'No Window' Batch\" -message \"There is no model loaded.\\n\\nPlease select one before trying to start a 'No Window' batch.\"");
		break;
	}

	// confirm overwriting current configuration
	cmd(inter, "button .ok -text Ok -command {set choice 1}");
	cmd(inter, "button .cancel -text Cancel -command {set choice 2}");
	cmd(inter, "label .war1 -text \"Starting 'No Window' batch for the model configuration:\"");
	sprintf(ch, "label .war2 -text \"%s\" -fg red", simul_name);
	cmd(inter, ch);
	sprintf(ch, "label .war3 -text \"Number of simulations: %d\"", sim_num);
	cmd(inter, ch);
	sprintf(ch, "label .war4 -text \"Time steps (max): %d\"", max_step);
	cmd(inter, ch);
	cmd(inter, "label .war5 -text \"Results file(s) (single simulation):\"");
	cmd(inter, "label .war7 -text \"Total file (last steps):\"");
	sprintf(ch, "label .war8 -text \"%s_%d_%d.tot\"", simul_name, seed, seed+sim_num-1);
	cmd(inter, ch);
	cmd(inter, "label .tosave -text \"\\nYou are going to overwrite the existing configuration file\\nand any results files in the destination folder\\n\"");
	
	if(sim_num>1)	// multiple runs case
	{
		sprintf(ch, "label .war6 -text \"from %s_%d.res%s to %s_%d.res%s\"", simul_name, seed, dozip ? ".gz" : "", simul_name, seed+sim_num-1);
		cmd(inter, ch);
		cmd(inter, "set wind \".war1 .war2 .war3 .war4 .war5 .war6 .war7 .war8 .tosave\"");
	}
	else			// single run case
	{
		sprintf(ch, "label .war6 -text \"%s_%d.res%s\"", simul_name, seed, dozip ? ".gz" : "");
		cmd(inter, ch);
		cmd(inter, "set wind \".war1 .war2 .war4 .war5 .war6 .war7 .war8 .tosave\"");
	}
	
	cmd(inter, "foreach i $wind {pack $i}; pack .ok .cancel; ");
	cmd(inter, "bind . <KeyPress-Return> {.ok invoke}; bind . <KeyPress-Escape> {.cancel invoke}");
	cmd(inter, "focus -force .");
	set_window_size();
	
	*choice=0;
	while(*choice==0)
		Tcl_DoOneEvent(0);
	
	cmd(inter, "bind . <KeyPress-Return> {}; bind . <KeyPress-Escape> {}");
	cmd(inter, "foreach i $wind {destroy $i}; destroy .ok .cancel");
	
	if(*choice==2)
		break;

	// save the current configuration
	for(n=r; n->up!=NULL; n=n->up);
	blueprint->empty();			    // update blueprint to consider last changes
	set_blueprint(blueprint, n);

	if(strlen(path)>0)
		sprintf(struct_file, "%s/%s.lsd", path, simul_name);
	else
		sprintf(struct_file, "%s.lsd", simul_name);
	f=fopen(struct_file, "w");
	if(f==NULL)
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Configuration file  cannot be opened.\n\nCheck if the file is set READ-ONLY.");
		break;
	}
	else							// run save procedure
	{
		strcpy(lab, "");
		for(cur=r; cur->up!=NULL; cur=cur->up);
		cur->save_struct(f,lab);
		fprintf(f, "\nDATA\n");
		cur->save_param(f);
		fprintf(f, "\nSIM_NUM %d\nSEED %d\nMAX_STEP %d\nEQUATION %s\n MODELREPORT %s\n", sim_num, seed, max_step, equation_name, name_rep);
		fprintf(f, "\nDESCRIPTION\n\n");
		save_description(cur, f);
		fprintf(f, "\nDOCUOBSERVE\n");
		for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
			if(cur_descr->observe=='y')     
				fprintf(f, "%s\n",cur_descr->label);
		fprintf(f, "\nEND_DOCUOBSERVE\n\n");
		fprintf(f, "\nDOCUINITIAL\n");
		for(cur_descr=descr; cur_descr!=NULL; cur_descr=cur_descr->next)
			if(cur_descr->initial=='y')     
				fprintf(f, "%s\n",cur_descr->label);
		fprintf(f, "\nEND_DOCUINITIAL\n\n");
		save_eqfile(f);
		fclose(f);
	}

	// check for existing NW executable
	sprintf(lab, "%s/%s", exec_path, exec_file);		// form full executable name
	cmd(inter, "if {$tcl_platform(platform) == \"windows\"} {set choice 1} {set choice 0}");
	if(*choice == 1)
	{
		// remove Windows extension, if present
		if((lab1 = strstr(lab, ".exe")) != NULL)
			lab1[0]='\0';
		else
			if((lab1 = strstr(lab, ".EXE")) != NULL) 
				lab1[0]='\0';
			
		strcat(lab, "NW.exe");							// add Windows ending
	}
	else
		strcat(lab, "NW");								// add Unix ending

	if ((f=fopen(lab, "rb")) == NULL) 
	{
		cmd(inter, "tk_messageBox -type ok -icon error -title \"Start 'No Window' Batch\" -message \"The executable file 'lsd_gnuNW' was not found.\n\nPlease create the required executable file using the option 'Model'/'Generate 'No Window' Version' in LMM first.\"");
		break;
	}
	fclose(f);
	
	// start the job
	if(strlen(path)>0)
		sprintf(msg, "set path \"%s\"", path);
	else
		sprintf(msg, "set path [pwd]");
	cmd(inter, msg);
	cmd(inter, "cd $path");

	if(*choice == 1)							// Windows?
		sprintf(msg, "exec %s -f %s >& %s.log  &", lab, struct_file, simul_name);
	else										// Unix
		sprintf(msg, "exec %s -f %s >& %s.log  &", lab, struct_file, simul_name);
    cmd(inter, msg);

	sprintf(msg, "tk_messageBox -type ok -icon info -title \"Start 'No Window' Batch\" -message \"The current configuration was started as a 'No Window' background job.\\n\\nThe results files are being created in the folder:\\n\\n$path\\n\\nCheck the '%s.log' file to see the results or use the command 'tail  -F  %s.log' in a shell/command prompt to follow simulation execution.\"", simul_name, simul_name);
	cmd(inter, msg);
break;

// toggle the state of the model structure windows
case 70:
	cmd(inter, "destroy .l .m");
break;

default:
sprintf(ch,"\nChoice %d not recognized\n",*choice);
plog(ch);
break;
}

*choice=0;
return r;
}


/****************************************************
CLEAN_DEBUG
****************************************************/
void clean_debug(object *n)
{
variable *cv;
object *co;
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 cv->debug='n';

for(cb=n->b; cb!=NULL; cb=cb->next)
for(co=cb->head; co!=NULL; co=co->next)
 clean_debug(co);
}

/****************************************************
CLEAN_SAVE
****************************************************/
void clean_save(object *n)
{
variable *cv;
object *co;
bridge *cb; 

for(cv=n->v; cv!=NULL; cv=cv->next)
 cv->save=0;
for(cb=n->b; cb!=NULL; cb=cb->next)
for(co=cb->head; co!=NULL; co=co->next)
 clean_save(co);
}


/****************************************************
SHOW_SAVE
****************************************************/
void show_save(object *n)
{
variable *cv;
object *co;
bridge *cb;

int app;
for(cv=n->v; cv!=NULL; cv=cv->next)
 {if(cv->save==1)
  {
   if(cv->param==1)
    sprintf(msg, "Object: %s \tParam:\t", n->label);
   else
    sprintf(msg, "Object: %s \tVar:\t", n->label);
   plog(msg);

   cmd(inter, "");
   sprintf(msg, ".log.text.text.internal insert end \"%s\" highlight", cv->label);
   cmd(inter, msg);
   plog("\n");

  }
 }

for(cb=n->b; cb!=NULL; cb=cb->next)
 {
 if(cb->head==NULL)
  co=blueprint->search(cb->blabel);
 else
  co=cb->head; 
 show_save(co);
 }
}



/****************************************************
SHOW_OBSERVE
****************************************************/
void show_observe(object *n)
{
variable *cv;
object *co;
description *cd;
int app;
bridge *cb;


for(cv=n->v; cv!=NULL; cv=cv->next)
 {
 cd=search_description(cv->label);
 if(cd!=NULL && cd->observe=='y')
  {
   if(cv->param==1)
    sprintf(msg, "Object: %s \tParam:\t", n->label);
   else
    sprintf(msg, "Object: %s \tVar:\t", n->label);
   plog(msg);

   cmd(inter, "");
   sprintf(msg, ".log.text.text.internal insert end \"%s (%lf)\" highlight", cv->label, cv->val[0]);
   cmd(inter, msg);
   plog("\n");

  }
 }

for(cb=n->b; cb!=NULL; cb=cb->next)
 {
 if(cb->head==NULL)
  co=blueprint->search(cb->blabel);
 else
  co=cb->head; 
 show_observe(co);
 }
}

/****************************************************
SHOW_INITIAL
****************************************************/
void show_initial(object *n)
{
variable *cv, *cv1;
object *co;
description *cd;
int app, i;
char s[1];
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 {
 cd=search_description(cv->label);
 if(cd!=NULL && cd->initial=='y')
  {
   if(cv->param==1)
    sprintf(msg, "Object: %s \tParam:\t", n->label);
   if(cv->param==0)
    sprintf(msg, "Object: %s \tVar:\t", n->label);
   if(cv->param==2)
    sprintf(msg, "Object: %s \tFunc:\t", n->label);
   plog(msg);

   cmd(inter, "");
   sprintf(msg, ".log.text.text.internal insert end \"%s \t\" highlight", cv->label);
   cmd(inter, msg);
   if(cd->init==NULL || strlen(cd->init)==0)
    {
    for(co=n; co!=NULL; co=co->hyper_next(co->label))
     {
      cv1=co->search_var(NULL,cv->label);
      sprintf(msg, " %g",cv1->val[0]);
      plog(msg);
     }
    }
   else
    {
     for(i=0; cd->init[i]!=0; i++)
      {
        switch(cd->init[i])
        {
        case '[': plog("\\\[");
                  break;
        case ']': plog("]");
                  break;
        case '"': plog("\\\"");
                  break;
        case '{': plog("\{");
                  break;
        default: sprintf(msg, "%c", cd->init[i]);
                 plog(msg);
                 break;          
                  
        }
  
      }
    
    } 
   plog("\n");

  }
 }

for(cb=n->b; cb!=NULL; cb=cb->next)
 {
 if(cb->head!=NULL)
  {co=cb->head; 
   show_initial(co);
  }
 } 
}


/****************************************************
CLEAN_PLOT
****************************************************/
void clean_plot(object *n)
{
variable *cv;
object *co;
bridge *cb;

for(cv=n->v; cv!=NULL; cv=cv->next)
 cv->plot=0;
 
for(cb=n->b; cb!=NULL; cb=cb->next)
for(co=cb->head; co!=NULL; co=co->next)
 clean_plot(co);
}

/****************************************************
WIPE_OUT
****************************************************/
void wipe_out(object *d)
{
object *cur;


cur=d->hyper_next(d->label);
if(cur!=NULL)
 wipe_out(cur);


delete_bridge(d);
}

/****************************************************
CHECK_LABEL
Control that the label l does not already exist in the model
****************************************************/
int check_label(char *l, object *r)
{
object *cur;
variable *cv;
bridge *cb;


if(!strcmp(l, r->label) )
 return 1;

for(cv=r->v; cv!=NULL; cv=cv->next)
 if(!strcmp(l, cv->label) )
  return 1;

for(cb=r->b; cb!=NULL; cb=cb->next)
{
 if(cb->head==NULL)
  cur=blueprint->search(cb->blabel);
 else
  cur=cb->head; 
 if(check_label(l, cur)==1)
   return 1;
} 
return 0;
}



void control_tocompute(object *r, char *l)
{
object *cur;
variable *cv;
bridge *cb;

 for(cv=r->v; cv!=NULL; cv=cv->next)
  {
  if(cv->save==1)
   {
   sprintf(msg, "tk_messageBox -type ok -title Warning -message \"Warning: item '%s' set to be saved, but will not be available, since object '%s' is set to be not computed.\"", cv->label, l);
   cmd(inter, msg);
   } 
   }


for(cb=r->b; cb!=NULL; cb=cb->next)
{
 if(cb->head==NULL)
  cur=blueprint->search(cb->blabel);
 else
  cur=cb->head; 
  control_tocompute(cur,l);
  } 
  

}

char *choose_object(void)
{
int done1;
char *ol;

Tcl_LinkVar(inter, "done1", (char *) &done1, TCL_LINK_INT);

cmd(inter, "toplevel .a");
cmd(inter, "wm protocol .a WM_DELETE_WINDOW { }");
cmd(inter, "wm transient .a .");
cmd(inter, "wm title .a \"\"");

cmd(inter, "label .a.l -text \"List of Objects\"");
cmd(inter, "pack .a.l");
cmd(inter, "frame .a.v");
cmd(inter, "scrollbar .a.v.v_scroll -command \".a.v.lb yview\"");
cmd(inter, "listbox .a.v.lb -selectmode single -yscroll \".a.v.v_scroll set\"");
cmd(inter, "pack .a.v.lb .a.v.v_scroll -side left -fill y");

insert_lb_object(root);

cmd(inter, "raise .a .; focus -force .a.v.lb; grab set .a");
cmd(inter, ".a.v.lb selection set 0");
cmd(inter, "set choice 0");

cmd(inter, "button .a.ok -text \" Ok \" -command {set done1 1}");

cmd(inter, "bind .a.v.lb <Double-1> {.a.ok invoke}");
cmd(inter, "pack .a.v .a.ok");
done1=0;
while(done1==0)
 Tcl_DoOneEvent(0);

cmd(inter, "set movelabel [.a.v.lb get [.a.v.lb curselection]]");
ol=(char *)Tcl_GetVar(inter, "movelabel",0);

cmd(inter, "grab release .a; destroy .a");
Tcl_UnlinkVar(inter,"done1");
return ol;
}


void insert_lb_object(object *r)
{
object *cur;
bridge *cb;

sprintf(msg, ".a.v.lb insert end %s", r->label);
cmd(inter, msg);

for(cb=r->b; cb!=NULL; cb=cb->next)
 {
  if(cb->head==NULL)
   cur=blueprint->search(cb->blabel);
  else
   cur=cb->head; 
  insert_lb_object(cur);
 }

}

void shift_var(int direction, char *vlab, object *r)
{
variable *cv, *cv1=NULL, *cv2=NULL;
if(direction==-1)
 {//shift up
  if(!strcmp(vlab, r->v->label))
   return; //variable already at the top
  if(!strcmp(vlab, r->v->next->label))
   {//second var, must become the head of the chain
    cv=r->v->next->next;//third
    cv1=r->v; //first
    r->v=r->v->next; //shifted up
    r->v->next=cv1;
    cv1->next=cv;
    return;
   }  
  for(cv=r->v; cv!=NULL; cv=cv->next)
   {
    if(!strcmp(vlab,cv->label) )
     {
      cv2->next=cv;
      cv1->next=cv->next;
      cv->next=cv1;
      return;
     }
    cv2=cv1;
    cv1=cv;
   }
 
 
 }
if(direction==1)
 {
  //move down
  if(!strcmp(vlab, r->v->label) )
   {//it the first
    if(r->v->next==NULL)
     return; //it is unique
    cv=r->v;//first
    cv1=cv->next->next;//third
    
    r->v=cv->next;//first is former second
    r->v->next=cv;//second is former first
    cv->next=cv1;//second points to third
    return;
   }
  for(cv=r->v; cv!=NULL; cv=cv->next)
   {
    
    if(!strcmp(vlab,cv->label) )
     {
      if(cv->next==NULL)
       return;//already at the end
      cv1->next=cv->next;
      cv->next=cv->next->next;
      cv1->next->next=cv;
      return;
     }
    cv1=cv;
   }
   
 } 
plog("\nError in move_var: should never reach this line\n"); 
}

void shift_desc(int direction, char *dlab, object *r)
{
bridge *cb, *cb1=NULL, *cb2=NULL;
if(direction==-1)
 {//shift up
  if(!strcmp(dlab, r->b->blabel))
   return; //object already at the top
  if(!strcmp(dlab, r->b->next->blabel))
   {//second var, must become the head of the chain
    cb=r->b->next->next;//third
    cb1=r->b; //first
    r->b=r->b->next; //shifted up
    r->b->next=cb1;
    cb1->next=cb;
    return;
   }  
  for(cb=r->b; cb!=NULL; cb=cb->next)
   {
    if(!strcmp(dlab,cb->blabel) )
     {
      cb2->next=cb;
      cb1->next=cb->next;
      cb->next=cb1;
      return;
     }
    cb2=cb1;
    cb1=cb;
   }
 
 
 }
if(direction==1)
 {
  //move down
  if(!strcmp(dlab, r->b->blabel) )
   {//it the first
    if(r->b->next==NULL)
     return; //it is unique
    cb=r->b;//first
    cb1=cb->next->next;//third
    
    r->b=cb->next;//first is former second
    r->b->next=cb;//second is former first
    cb->next=cb1;//second points to third
    return;
   }
  for(cb=r->b; cb!=NULL; cb=cb->next)
   {
    
    if(!strcmp(dlab,cb->blabel) )
     {
      if(cb->next==NULL)
       return;//already at the end
      cb1->next=cb->next;
      cb->next=cb->next->next;
      cb1->next->next=cb;
      return;
     }
    cb1=cb;
   }
   
 } 
plog("\nError in shift_desc: should never reach this line\n"); 
}

/*
	Ask user to discard changes in configuration, if applicable
	Returns: 0: abort, 1: continue without saving
*/

bool discard_change( bool checkSense )
{
	bool discard;
	
	// just run or nothing to save?
	if ( actual_steps > 0 || ! unsavedChange )
		discard = true;					// simply discard configuration
	else
	{
		Tcl_SetVar( inter, "filename", simul_name , 0 );
		// ask the user what to do
		cmd( inter, "set answer [tk_messageBox -type okcancel -default cancel -icon warning -title \"Discard changes?\" -message \"Recent changes to configuration '$filename' have not been saved!\n\nDo you want to discard them and continue?\"]" );
		cmd( inter, "if { $answer == \"ok\" } { set ans 1 } { set ans 0 }" );  
		const char *ans = Tcl_GetVar( inter, "ans", 0 );
		discard = atoi( ans );
	}
	
 	// check also for sensitivity data changed
	if ( discard && ( ! checkSense || discard_sense( ) ) )
		return true;
	else
		return false;
}

/*
	Ask user to discard changes in sensitivity data, if applicable
	Returns: 0: abort, 1: continue without saving
*/

bool discard_sense( void )
{
	// nothing to save?
	if ( rsense == NULL || ! unsavedSense )
		return true;					// simply discard

	// ask the user what to do
	cmd( inter, "set answer [tk_messageBox -type okcancel -default cancel -icon warning -title \"Discard changes?\" -message \"Recent changes to sensitivity data have not been saved!\n\nDo you want to discard them and continue?]" );
	cmd( inter, "if { $answer == \"ok\" } { set ans 1 } { set ans 0 }" );  
	const char *ans = Tcl_GetVar( inter, "ans", 0 );
	return atoi( ans );
}

// Entry point function for access from the Tcl interpreter
int Tcl_discard_change( ClientData cdata, Tcl_Interp *inter, int argc, const char *argv[] )
{
	if ( discard_change( true ) == 1 )
		inter->result = ( char * ) "ok";
	else
		inter->result = ( char * ) "cancel";
	return TCL_OK;
}