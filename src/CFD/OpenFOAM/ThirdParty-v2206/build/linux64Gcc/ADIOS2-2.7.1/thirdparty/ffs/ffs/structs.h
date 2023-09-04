#ifndef __STRUCTS_H
#define __STRUCTS_H
struct list_struct {
    sm_ref node;
    struct list_struct *next;
};

typedef enum {
    cod_label_statement,
    cod_designator,
    cod_field_ref,
    cod_struct_type_decl,
    cod_cast,
    cod_selection_statement,
    cod_assignment_expression,
    cod_conditional_operator,
    cod_comma_expression,
    cod_reference_type_decl,
    cod_jump_statement,
    cod_initializer_list,
    cod_iteration_statement,
    cod_enumerator,
    cod_declaration,
    cod_compound_statement,
    cod_identifier,
    cod_expression_statement,
    cod_field,
    cod_element_ref,
    cod_array_type_decl,
    cod_return_statement,
    cod_operator,
    cod_type_specifier,
    cod_initializer,
    cod_subroutine_call,
    cod_enum_type_decl,
    cod_constant,
    cod_last_node_type
} cod_node_type;

typedef struct {
    sm_ref statement;
    int cg_label;
    char* name;
} label_statement;

typedef struct {
    char* id;
    sm_ref expression;
} designator;

typedef struct {
    srcpos lx_srcpos;
    char* lx_field;
    sm_ref struct_ref;
    sm_ref sm_field_ref;
} field_ref;

typedef struct {
    enc_info encode_info;
    srcpos lx_srcpos;
    char* id;
    int cg_size;
    sm_list fields;
} struct_type_decl;

typedef struct {
    sm_ref expression;
    sm_ref sm_complex_type;
    sm_list type_spec;
    srcpos lx_srcpos;
    int cg_type;
} cast;

typedef struct {
    sm_ref then_part;
    srcpos lx_srcpos;
    sm_ref conditional;
    sm_ref else_part;
} selection_statement;

typedef struct {
    sm_ref left;
    int cg_type;
    operator_t op;
    sm_ref right;
    srcpos lx_srcpos;
} assignment_expression;

typedef struct {
    int result_type;
    sm_ref condition;
    sm_ref e1;
    srcpos lx_srcpos;
    sm_ref e2;
} conditional_operator;

typedef struct {
    sm_ref right;
    srcpos lx_srcpos;
    sm_ref left;
} comma_expression;

typedef struct {
    int cg_referenced_size;
    char* name;
    sm_list type_spec;
    sm_ref freeable_complex_referenced_type;
    sm_ref sm_complex_referenced_type;
    int cg_referenced_type;
    srcpos lx_srcpos;
    int kernel_ref;
} reference_type_decl;

typedef struct {
    sm_ref sm_target_stmt;
    srcpos lx_srcpos;
    char* goto_target;
    int continue_flag;
} jump_statement;

typedef struct {
    sm_list initializers;
} initializer_list;

typedef struct {
    sm_ref test_expr;
    int cg_end_label;
    sm_ref iter_expr;
    sm_ref statement;
    int cg_iter_label;
    sm_ref init_expr;
    srcpos lx_srcpos;
    sm_ref post_test_expr;
} iteration_statement;

typedef struct {
    char* id;
    sm_ref const_expression;
    int enum_value;
} enumerator;

typedef struct {
    int static_var;
    int addr_taken;
    void* cg_address;
    sm_list params;
    void* closure_id;
    int cg_oprnd;
    int is_typedef;
    int const_var;
    sm_ref sm_complex_type;
    srcpos lx_srcpos;
    int is_subroutine;
    int varidiac_subroutine_param_count;
    int param_num;
    sm_list type_spec;
    char* id;
    sm_ref freeable_complex_type;
    int is_extern;
    sm_ref init_value;
    int cg_type;
} declaration;

typedef struct {
    sm_list statements;
    sm_list decls;
} compound_statement;

typedef struct {
    sm_ref sm_declaration;
    srcpos lx_srcpos;
    int cg_type;
    char* id;
} identifier;

typedef struct {
    sm_ref expression;
} expression_statement;

typedef struct {
    int cg_type;
    int cg_size;
    sm_ref sm_complex_type;
    int cg_offset;
    sm_ref freeable_complex_type;
    sm_list type_spec;
    char* string_type;
    char* name;
} field;

typedef struct {
    sm_ref sm_complex_element_type;
    srcpos lx_srcpos;
    sm_ref sm_containing_structure_ref;
    sm_ref array_ref;
    sm_ref expression;
    int cg_element_type;
    int this_index_dimension;
} element_ref;

typedef struct {
    sm_list type_spec;
    sm_ref freeable_complex_element_type;
    sm_ref size_expr;
    sm_ref sm_dynamic_size;
    int cg_element_size;
    int cg_static_size;
    dimen_p dimensions;
    sm_ref sm_complex_element_type;
    int static_var;
    int cg_element_type;
    sm_ref element_ref;
    srcpos lx_srcpos;
} array_type_decl;

typedef struct {
    int cg_func_type;
    srcpos lx_srcpos;
    sm_ref expression;
} return_statement;

typedef struct {
    int operation_type;
    sm_ref left;
    int result_type;
    sm_ref right;
    operator_t op;
    srcpos lx_srcpos;
} operator;

typedef struct {
    sm_ref created_type_decl;
    srcpos lx_srcpos;
    int token;
} type_specifier;

typedef struct {
    sm_ref initializer;
    sm_list designation;
} initializer;

typedef struct {
    sm_ref sm_func_ref;
    sm_list arguments;
    srcpos lx_srcpos;
} subroutine_call;

typedef struct {
    char* id;
    srcpos lx_srcpos;
    sm_list enums;
} enum_type_decl;

typedef struct {
    srcpos lx_srcpos;
    int token;
    char* freeable_name;
    char* const_val;
} constant;

typedef union {
   label_statement label_statement;
   designator designator;
   field_ref field_ref;
   struct_type_decl struct_type_decl;
   cast cast;
   selection_statement selection_statement;
   assignment_expression assignment_expression;
   conditional_operator conditional_operator;
   comma_expression comma_expression;
   reference_type_decl reference_type_decl;
   jump_statement jump_statement;
   initializer_list initializer_list;
   iteration_statement iteration_statement;
   enumerator enumerator;
   declaration declaration;
   compound_statement compound_statement;
   identifier identifier;
   expression_statement expression_statement;
   field field;
   element_ref element_ref;
   array_type_decl array_type_decl;
   return_statement return_statement;
   operator operator;
   type_specifier type_specifier;
   initializer initializer;
   subroutine_call subroutine_call;
   enum_type_decl enum_type_decl;
   constant constant;
} sm_union;

struct sm_struct {
    cod_node_type node_type;
    int visited;
    sm_union node;
};
#endif
extern sm_ref cod_new_label_statement();
extern sm_ref cod_new_designator();
extern sm_ref cod_new_field_ref();
extern sm_ref cod_new_struct_type_decl();
extern sm_ref cod_new_cast();
extern sm_ref cod_new_selection_statement();
extern sm_ref cod_new_assignment_expression();
extern sm_ref cod_new_conditional_operator();
extern sm_ref cod_new_comma_expression();
extern sm_ref cod_new_reference_type_decl();
extern sm_ref cod_new_jump_statement();
extern sm_ref cod_new_initializer_list();
extern sm_ref cod_new_iteration_statement();
extern sm_ref cod_new_enumerator();
extern sm_ref cod_new_declaration();
extern sm_ref cod_new_compound_statement();
extern sm_ref cod_new_identifier();
extern sm_ref cod_new_expression_statement();
extern sm_ref cod_new_field();
extern sm_ref cod_new_element_ref();
extern sm_ref cod_new_array_type_decl();
extern sm_ref cod_new_return_statement();
extern sm_ref cod_new_operator();
extern sm_ref cod_new_type_specifier();
extern sm_ref cod_new_initializer();
extern sm_ref cod_new_subroutine_call();
extern sm_ref cod_new_enum_type_decl();
extern sm_ref cod_new_constant();
typedef void (*cod_apply_func)(sm_ref node, void *data);
typedef void (*cod_apply_list_func)(sm_list list, void *data);
extern void cod_apply(sm_ref node, cod_apply_func pre_func, cod_apply_func post_func, cod_apply_list_func list_func, void *data);
extern void cod_print(sm_ref node);
extern void cod_free(sm_ref node);
extern void cod_free_list(sm_list list, void *junk);
extern void cod_rfree(sm_ref node);
extern void cod_rfree_list(sm_list list, void *junk);
extern sm_ref cod_copy(sm_ref node);
extern sm_list cod_copy_list(sm_list list);
extern sm_list cod_append_list(sm_list list1, sm_list list2);
extern srcpos cod_get_srcpos(sm_ref expr);
