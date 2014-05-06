#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Eo.h>
#include <Emodel.h>
#include <Elementary.h>

#include "elm_view_form.h"
#include "elm_view_list.h"

#include "elm_priv.h"
#include <assert.h>

typedef struct _Elm_View_Form_Private Elm_View_Form_Private;
typedef struct _Elm_View_Form_Item Elm_View_Form_Item;
typedef struct _Elm_View_Form_Widget Elm_View_Form_Widget;
typedef Eina_List Eina_List_Properties;
 
struct _Elm_View_Form_Widget
{
   const char *widget_name;
   Evas_Object *widget_obj;
};

struct _Elm_View_Form_Item
{
   const char *propname;
   Eina_List *widget_list;
};

struct _Elm_View_Form_Private
{
   Eo *model_obj;
   Eina_Value *properties;
   const char *model_name;
   Eina_List *l;
   Eina_List_Properties *lprop;
};


/**
 * @brief Helper functions
 */
static Eina_Bool
_elm_view_widget_add(Elm_View_Form_Private *priv, 
                           const char *widget_name, char *propname, Evas_Object *widget_obj)
{
   /**
    * First call
    */
   if(!priv->l) 
     {
        Elm_View_Form_Item *item = calloc(1, sizeof(Elm_View_Form_Item));
        EINA_SAFETY_ON_NULL_RETURN_VAL(item, EINA_FALSE);

        Elm_View_Form_Widget *widget = calloc(1, sizeof(Elm_View_Form_Widget));
        EINA_SAFETY_ON_NULL_RETURN_VAL(widget, EINA_FALSE);

        widget->widget_name = widget_name;
        widget->widget_obj = widget_obj;

        item->propname = propname;
        item->widget_list = eina_list_append(item->widget_list, widget);
        priv->l = eina_list_append(priv->l, item);
        //fprintf(stdout, "[%s:%d] Added item '%s' with '%s' from object '%p'\n", __FUNCTION__, __LINE__, item->propname, widget->widget_name, widget->widget_obj);
     } 
   else
     {
        Elm_View_Form_Item *nItem;
        Elm_View_Form_Widget *nWidget;
        for(Eina_List *l = priv->l; l; l = eina_list_next(l))
          {
             nItem = eina_list_data_get(l);

             /**
              * List is already created and property already exists so we append a new Evas_Object
              */
             if(!strncmp(nItem->propname, propname, strlen(nItem->propname)))
               {
                  nWidget = calloc(1, sizeof(Elm_View_Form_Widget));
                  EINA_SAFETY_ON_NULL_RETURN_VAL(nWidget, EINA_FALSE);

                  nWidget->widget_name = widget_name;
                  nWidget->widget_obj = widget_obj;
                  nItem->widget_list = eina_list_append(nItem->widget_list, nWidget);
                  //fprintf(stdout, "[%s:%d] Added item '%s' with '%s' from object '%p'\n", __FUNCTION__, __LINE__, nItem->propname, nWidget->widget_name, nWidget->widget_obj);
               }

             /**
              * List is already created and property is new so we create a new tupple
              */
             else
               {
                  nItem = calloc(1, sizeof(Elm_View_Form_Item));
                  EINA_SAFETY_ON_NULL_RETURN_VAL(nItem, EINA_FALSE);

                  nWidget = calloc(1, sizeof(Elm_View_Form_Widget));
                  EINA_SAFETY_ON_NULL_RETURN_VAL(nWidget, EINA_FALSE);

                  nWidget->widget_name = widget_name;
                  nWidget->widget_obj = widget_obj;

                  nItem->propname = propname;
                  nItem->widget_list = eina_list_append(nItem->widget_list, nWidget);
                  //fprintf(stdout, "[%s:%d] Added item '%s' with '%s' from object '%p'\n", __FUNCTION__, __LINE__, nItem->propname, nWidget->widget_name, nWidget->widget_obj);

                  l = eina_list_append(l, nItem);
                  l = eina_list_next(l);
               }
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_elm_view_form_property_find(Eina_List_Properties *l, const char *propname)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(l, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(propname, EINA_FALSE);

   for(Eina_List_Properties *_l = l; _l; _l = eina_list_next(_l))
     {
        const char *p = eina_list_data_get(_l);
        EINA_SAFETY_ON_NULL_RETURN_VAL(p, EINA_FALSE);
        if(!strncmp(p, propname, strlen(propname)))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

/**
 * @brief Widget/Property event callbacks
 */
static Eina_Bool
_elm_view_form_property_change_cb(void *data, Eo *obj EINA_UNUSED, 
                                  const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_View_Form_Private *priv = (Elm_View_Form_Private *)data;
   Emodel_Property_EVT *evt = event_info;

   EINA_SAFETY_ON_NULL_RETURN_VAL(evt, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EINA_FALSE);

   for(Eina_List *l = priv->l; l; l = eina_list_next(l))
     {
        Elm_View_Form_Item *nItem = eina_list_data_get(l);

        /** 
         * Is current property on the list?
         */ 
         if(_elm_view_form_property_find(priv->lprop, nItem->propname) == EINA_FALSE)
           return EINA_FALSE;

        for(Eina_List *lwidget = nItem->widget_list; lwidget; lwidget = eina_list_next(lwidget))
          {
             Elm_View_Form_Widget *nWidget = eina_list_data_get(lwidget);
             if(!nWidget || !nWidget->widget_name || !nWidget->widget_obj) continue;

             if(!strncmp(nWidget->widget_name, "Elm_Label", strlen("Elm_Label")))
               {
                  char *text = eina_value_to_string(evt->value);
                  if(text) elm_object_text_set(nWidget->widget_obj, text);
               }
             else if(!strncmp(nWidget->widget_name, "Elm_Clock", strlen("Elm_Clock")))
               {
                  //TODO: implement
               }
          }
     }
   return EINA_TRUE;
}


/**
 * @brief Elm View Form Class
 */
EAPI Eo_Op ELM_VIEW_FORM_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_VIEW_FORM_CLASS
#define MY_CLASS_NAME "View Form"


/**
 * @brief constructor
 */
static void
_elm_view_form_constructor(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Elm_View_Form_Private *priv = (Elm_View_Form_Private *)class_data;
   Eo *model_obj = va_arg(*list, Eo *);
   
   eo_do_super(obj, MY_CLASS, eo_constructor());

   EINA_SAFETY_ON_NULL_RETURN(model_obj);

   priv->model_obj = model_obj;
   priv->model_name = eo_class_name_get(priv->model_obj);
   eo_do(priv->model_obj, eo_event_callback_add(EMODEL_PROPERTY_CHANGE_EVT, _elm_view_form_property_change_cb, priv));
}

/**
 * @brief destructor
 */
static void
_elm_view_form_destructor(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   //TODO: cleanup/free data
   eo_do_super(obj, MY_CLASS, eo_destructor());
}


static void
_elm_view_form_property_add(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Elm_View_Form_Private *priv = (Elm_View_Form_Private *)class_data;
   char *tmp = va_arg(*list, const char *);
   char *propname;

   EINA_SAFETY_ON_NULL_RETURN(tmp);
   propname = calloc(1, strlen(tmp)+1);
   EINA_SAFETY_ON_NULL_RETURN(propname);

   priv->lprop = eina_list_append(priv->lprop, strncpy(propname, tmp, strlen(tmp)));
}

static void
_elm_view_form_widget_add(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Eina_Bool status;
   Elm_View_Form_Private *priv = (Elm_View_Form_Private *)class_data;
   char *propname = va_arg(*list, const char *);
   Evas_Object *evas = va_arg(*list, Evas_Object *);
   const char *objname = eo_class_name_get(evas);

   EINA_SAFETY_ON_NULL_RETURN(propname);
   EINA_SAFETY_ON_NULL_RETURN(evas);

   status = _elm_view_widget_add(priv, objname, propname, evas);
   EINA_SAFETY_ON_FALSE_RETURN(status);
}
      
static void
_view_form_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_descs[] = {
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _elm_view_form_constructor),
      EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _elm_view_form_destructor),
      EO_OP_FUNC(ELM_VIEW_FORM_ID(ELM_OBJ_VIEW_FORM_SUB_ID_WIDGET_PROPERTY_ADD), _elm_view_form_property_add),
      EO_OP_FUNC(ELM_VIEW_FORM_ID(ELM_OBJ_VIEW_FORM_SUB_ID_WIDGET_ADD), _elm_view_form_widget_add),
      EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_descs);
}

static const Eo_Op_Description op_descs[] = {
   EO_OP_DESCRIPTION(ELM_OBJ_VIEW_FORM_SUB_ID_WIDGET_PROPERTY_ADD, "Add property pair"),
   EO_OP_DESCRIPTION(ELM_OBJ_VIEW_FORM_SUB_ID_WIDGET_ADD, "Add new widget"),
   EO_OP_DESCRIPTION_SENTINEL
};

static Eo_Class_Description view_form_class_descs = {
   EO_VERSION,
   MY_CLASS_NAME,
   EO_CLASS_TYPE_REGULAR,
   EO_CLASS_DESCRIPTION_OPS(&ELM_VIEW_FORM_BASE_ID, op_descs, ELM_OBJ_VIEW_FORM_SUB_ID_LAST),
   NULL,
   sizeof(Elm_View_Form_Private),
   _view_form_class_constructor,
   NULL
};

EO_DEFINE_CLASS(elm_obj_view_form_class_get, &view_form_class_descs, EO_BASE_CLASS, NULL);
