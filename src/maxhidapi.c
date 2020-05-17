#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include "hidapi.h"
#include "ext.h"
#include "ext_obex.h"

typedef struct _maxhidapi
{
    t_object s_obj;     // t_object header
    hid_device * device;
    struct hid_device_info * devices;
    unsigned short vendor_id;
    unsigned short product_id;
    void * outlet;
} t_maxhidapi;

static t_class * s_maxhidapi_class;

//function prototypes
void maxhidapi_open(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv);
void maxhidapi_write(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv);
void maxhidapi_read(t_maxhidapi * x, long report_id, long length);
void maxhidapi_close(t_maxhidapi * x);
void maxhidapi_get_product_string(t_maxhidapi * x);
void maxhidapi_get_manufacturer_string(t_maxhidapi * x);
void maxhidapi_get_serial_number_string(t_maxhidapi * x);
void maxhidapi_get_indexed_string(t_maxhidapi * x, long index);
void maxhidapi_error(t_maxhidapi * x);
void maxhidapi_set_nonblocking(t_maxhidapi * x, long nonblock);
void maxhidapi_enumerate(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv);
void maxhidapi_open_path(t_maxhidapi * x, t_symbol * s);
void maxhidapi_send_feature_report(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv);
void maxhidapi_get_feature_report(t_maxhidapi * x, long report_id, long length);
void maxhidapi_get_input_report(t_maxhidapi * x, long report_id, long length);

void * maxhidapi_new(){
    t_maxhidapi * x = (t_maxhidapi *)object_alloc(s_maxhidapi_class);
    x->outlet = outlet_new((t_object *) x, NULL);
    hid_init();
    x->device = NULL;
    x->devices = NULL;
    return x;
}

void maxhidapi_free(t_maxhidapi * x){
    if(x->device != NULL){
        hid_close(x->device);
    }
    if(x->devices != NULL){
        hid_free_enumeration(x->devices);
    }
    hid_exit();
    return;
}

void ext_main(void * r){
    t_class * c;
    c = class_new("hidapi", (method)maxhidapi_new, (method)maxhidapi_free, sizeof(t_maxhidapi), NULL, 0);
    class_addmethod(c, (method)maxhidapi_open, "open", A_GIMME, 0);
    class_addmethod(c, (method)maxhidapi_write, "write", A_GIMME, 0);
    class_addmethod(c, (method)maxhidapi_read, "read", A_LONG, A_LONG, 0);
    class_addmethod(c, (method)maxhidapi_close, "close", 0);
    class_addmethod(c, (method)maxhidapi_get_product_string, "get_product_string", 0);
    class_addmethod(c, (method)maxhidapi_get_manufacturer_string, "get_manufacturer_string", 0);
    class_addmethod(c, (method)maxhidapi_get_serial_number_string, "get_serial_number_string", 0);
    class_addmethod(c, (method)maxhidapi_get_indexed_string, "get_indexed_string", A_LONG, 0);
    class_addmethod(c, (method)maxhidapi_error, "error", 0);
    class_addmethod(c, (method)maxhidapi_set_nonblocking, "set_nonblocking", A_LONG, 0);
    class_addmethod(c, (method)maxhidapi_enumerate, "enumerate", A_GIMME, 0);
    class_addmethod(c, (method)maxhidapi_open_path, "open_path", A_SYM, 0);
    class_addmethod(c, (method)maxhidapi_send_feature_report, "send_feature_report", A_GIMME, 0);
    class_addmethod(c, (method)maxhidapi_get_feature_report, "get_feature_report", A_LONG, A_LONG, 0);
    class_addmethod(c, (method)maxhidapi_get_input_report, "get_input_report", A_LONG, A_LONG, 0);
    class_register(CLASS_BOX, c);
    s_maxhidapi_class = c;
}

void maxhidapi_open(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    if(x->device != NULL){
        maxhidapi_close(x);
    }
    hid_device * device;
    unsigned short vendor_id = 0;
    unsigned short product_id = 0;
    if(argc == 1){
        if(x->devices == NULL){
            post("Devices not enumerated (send enumerate)");
            return;
        }
        struct hid_device_info * cur_dev;
        cur_dev = x->devices;
        int i = 1;
        long index = atom_getlong(argv);
        while(cur_dev){
            if(index == i){
                device = hid_open_path(cur_dev->path);
                vendor_id = cur_dev->vendor_id;
                product_id = cur_dev->product_id;
                break;
            }
            cur_dev = cur_dev->next;
            ++i;
        }
        if(device == NULL){
            post("Error opening device");
        }
        else{
            x->device = device;
            x->vendor_id = vendor_id;
            x->product_id = product_id;
        }
    }
    if(argc == 2){
        vendor_id = (unsigned short)atom_getlong(argv);
        product_id = (unsigned short)atom_getlong(argv + 1);
        device = hid_open(vendor_id, product_id, NULL);
        if(device == NULL){
            post("Error opening device");
        }
        else{
            x->device = device;
            x->vendor_id = vendor_id;
            x->product_id = product_id;
        }
    }
    else if(argc == 3){
        vendor_id = (unsigned short)atom_getlong(argv);
        product_id = (unsigned short)atom_getlong(argv + 1);
        char * serial_number_c = atom_getsym(argv + 2)->s_name;
        size_t length = strlen(serial_number_c);
        wchar_t serial_number_w[length + 1];
        mbstowcs_s(NULL, serial_number_w, length+1, serial_number_c, length);
        device = hid_open(vendor_id, product_id, serial_number_w);
        if(device == NULL){
            post("Error opening device");
        }
        else{
            x->device = device;
            x->vendor_id = vendor_id;
            x->product_id = product_id;
        }
    }
}

void maxhidapi_write(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    if(x->device != NULL){
        int i;
        unsigned char length = (unsigned char)atom_getlong(argv);
        unsigned char * buffer = NULL;
        while(buffer == NULL){
            unsigned char * buffer = (unsigned char *)calloc(length, sizeof(unsigned char));
        }
        for(i = 0; i < argc - 1; i++){
            buffer[i] = (unsigned char)atom_getlong(argv + 1 + i);
        }
        int result = hid_write(x->device, buffer, length);
        free(buffer);
        t_atom result_a;
        atom_setlong(&result_a, result);
        outlet_anything(x->outlet, gensym("write"), 1, &result_a);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_read(t_maxhidapi * x, long report_id, long length){
    if(x->device != NULL){
        unsigned char buffer[length];
        if(report_id >= 0){
            buffer[0] = report_id;
        }
        int result = hid_read(x->device, buffer, length);
        if(result >= 0){
            t_atom argv[result];
            int i;
            for(i = 0; i < result; i++){
                atom_setlong(argv + i, buffer[i]);
            }
            outlet_anything(x->outlet, gensym("read"), result, argv);
        }
        else{
            t_atom argv;
            atom_setlong(&argv, result);
            outlet_anything(x->outlet, gensym("read"), 1, &argv);
        }
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_close(t_maxhidapi * x){
    if(x->device != NULL){
        t_atom argv[2];
        atom_setlong(argv, x->vendor_id);
        atom_setlong(argv + 1, x->product_id);
        hid_close(x->device);
        x->device = NULL;
        x->vendor_id = 0;
        x->product_id = 0;
        outlet_anything(x->outlet, gensym("close"), 2, argv);
    }
}

void maxhidapi_get_product_string(t_maxhidapi * x){
    if(x->device != NULL){
        wchar_t product_string_w[64];
        int result = hid_get_product_string(x->device, product_string_w, 64);
        char product_string_c[64];
        if(result == 0){
            wcstombs(product_string_c, product_string_w, 64);
            t_atom product_string_a;
            atom_setsym(&product_string_a, gensym(product_string_c));
            outlet_anything(x->outlet, gensym("get_product_string"), 1, &product_string_a);
        }
        else{
            post("Error getting product string");
        }
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_manufacturer_string(t_maxhidapi * x){
    if(x->device != NULL){
        wchar_t manufacturer_string_w[64];
        int result = hid_get_manufacturer_string(x->device, manufacturer_string_w, 64);
        char manufacturer_string_c[64];
        if(result == 0){
            wcstombs(manufacturer_string_c, manufacturer_string_w, 64);
            t_atom manufacturer_string_a;
            atom_setsym(&manufacturer_string_a, gensym(manufacturer_string_c));
            outlet_anything(x->outlet, gensym("get_manufacturer_string"), 1, &manufacturer_string_a);
        }
        else{
            post("Error getting manufacturer string");
        }
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_serial_number_string(t_maxhidapi * x){
    if(x->device != NULL){
        wchar_t serial_number_string_w[64];
        int result = hid_get_serial_number_string(x->device, serial_number_string_w, 64);
        char serial_number_string_c[64];
        if(result == 0){
            wcstombs(serial_number_string_c, serial_number_string_w, 64);
            t_atom serial_number_string_a;
            atom_setsym(&serial_number_string_a, gensym(serial_number_string_c));
            outlet_anything(x->outlet, gensym("get_serial_number_string"), 1, &serial_number_string_a);
        }
        else{
            post("Error getting serial number string");
        }
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_indexed_string(t_maxhidapi * x, long index){
    if(x->device != NULL){
        wchar_t indexed_string_w[64];
        int result = hid_get_indexed_string(x->device, index, indexed_string_w, 64);
        char indexed_string_c[64];
        if(result == 0){
            wcstombs(indexed_string_c, indexed_string_w, 64);
            t_atom indexed_string_a;
            atom_setsym(&indexed_string_a, gensym(indexed_string_c));
            outlet_anything(x->outlet, gensym("get_indexed_string"), 1, &indexed_string_a);
        }
        else{
            post("Error getting indexed string");
        }
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_error(t_maxhidapi * x){
    if(x->device != NULL){
        const wchar_t * error_w = hid_error(x->device);
        int length = wcslen(error_w);
        char error_c[length];
        wcstombs(error_c, error_w, length);
        t_atom error_a;
        atom_setsym(&error_a, gensym(error_c));
        outlet_anything(x->outlet, gensym("error"), 1, &error_a);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_set_nonblocking(t_maxhidapi * x, long nonblock){
    if(x->device != NULL){
        int result = hid_set_nonblocking(x->device, nonblock);
        t_atom argv;
        atom_setlong(&argv, result);
        outlet_anything(x->outlet, gensym("set_nonblocking"), 1, &argv);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_enumerate(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    unsigned short vendor_id = 0, product_id = 0;
    if (argc == 1){
        vendor_id = (unsigned short)atom_getlong(argv);
    }
    else if (argc == 2){
        vendor_id = (unsigned short)atom_getlong(argv);
        product_id = (unsigned short)atom_getlong(argv + 1);
    }
    struct hid_device_info * cur_dev;
    x->devices = hid_enumerate(vendor_id, product_id);
    cur_dev = x->devices;
    int i = 1;
    while(cur_dev){
        post("%d", i);
        post("Device Found  type: %04hx %04hx  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
        post("  Manufacturer: %ls", cur_dev->manufacturer_string);
        post("  Product:      %ls", cur_dev->product_string);
        post("  Release:      %hx", cur_dev->release_number);
        post("  Interface:    %d",  cur_dev->interface_number);
        post("  Usage (page): 0x%hx (0x%hx)", cur_dev->usage, cur_dev->usage_page);
        post("  Path:");
        post("%s", cur_dev->path);
        post(" ");
        cur_dev = cur_dev->next;
        ++i;
    }
}

void maxhidapi_open_path(t_maxhidapi * x, t_symbol * s){
    if(x->device != NULL){
        maxhidapi_close(x);
    }
    hid_device * device;
    const char * path = s->s_name;
    device = hid_open_path(path);
    if(device == NULL){
        post("Error opening device");
    }
    else{
        struct hid_device_info * cur_dev;
        cur_dev = x->devices;
        while(cur_dev){
            if(cur_dev->path == path){
                x->device = device;
                x->vendor_id = cur_dev->vendor_id;
                x->product_id = cur_dev->product_id;
                break;
            }
            cur_dev = cur_dev->next;
        }
        if(x->device == NULL){
            post("Error opening device");
        }
    }
}

void maxhidapi_send_feature_report(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    if(x->device != NULL){
        int i;
        unsigned char length = (unsigned char)atom_getlong(argv);
        unsigned char * buffer = NULL;
        while(buffer == NULL){
            unsigned char * buffer = (unsigned char *)calloc(length, sizeof(unsigned char));
        }
        for(i = 0; i < argc - 1; i++){
            buffer[i] = (unsigned char)atom_getlong(argv + 1 + i);
        }
        int result = hid_send_feature_report(x->device, buffer, length);
        free(buffer);
        t_atom result_a;
        atom_setlong(&result_a, result);
        outlet_anything(x->outlet, gensym("send_feature_report"), 1, &result_a);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_feature_report(t_maxhidapi * x, long report_id, long length){
    if(x->device != NULL){
        unsigned char buffer[length];
        buffer[0] = report_id;
        int result = hid_get_feature_report(x->device, buffer, length);
        if(result >= 0){
            t_atom argv[result];
            int i;
            for(i = 0; i < result; i++){
                atom_setlong(argv + i, buffer[i]);
            }
            outlet_anything(x->outlet, gensym("get_feature_report"), result, argv);
        }
        else{
            t_atom argv;
            atom_setlong(&argv, result);
            outlet_anything(x->outlet, gensym("get_feature_report"), 1, &argv);
        }
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_input_report(t_maxhidapi * x, long report_id, long length){
    if(x->device != NULL){
        unsigned char buffer[length];
        buffer[0] = report_id;
        int result = hid_get_input_report(x->device, buffer, length);
        if(result >= 0){
            t_atom argv[result];
            int i;
            for(i = 0; i < result; i++){
                atom_setlong(argv + i, buffer[i]);
            }
            outlet_anything(x->outlet, gensym("get_input_report"), result, argv);
        }
        else{
            t_atom argv;
            atom_setlong(&argv, result);
            outlet_anything(x->outlet, gensym("get_input_report"), 1, &argv);
        }
    }
    else{
        post("Not connected");
    }
}