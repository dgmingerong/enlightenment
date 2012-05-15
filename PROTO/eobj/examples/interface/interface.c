#include "Eo.h"
#include "interface.h"
#include "simple.h"

#include "config.h"

EAPI Eo_Op INTERFACE_BASE_ID = 0;

#define MY_CLASS INTERFACE_CLASS

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION_CONST(INTERFACE_SUB_ID_AB_SUM_GET, "i", "Get the sum of a and b."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     "Interface",
     EO_CLASS_TYPE_INTERFACE,
     EO_CLASS_DESCRIPTION_OPS(&INTERFACE_BASE_ID, op_desc, INTERFACE_SUB_ID_LAST),
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL
};

EO_DEFINE_CLASS(interface_class_get, &class_desc, NULL, NULL)

