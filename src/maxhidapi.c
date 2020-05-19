/**
 * 
 *  maxhidapi Max wrapper for the hidapi.
 *  Author: Malik Enes Safak
 *   Email: e.maliksafak@gmail.com
 *     Web: https://www.github.com/NullMember
 * License: MIT
 * 
 */

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
void maxhidapi_read(t_maxhidapi * x, long length, long report_id);
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
void maxhidapi_get_feature_report(t_maxhidapi * x, long length, long report_id);
void maxhidapi_get_input_report(t_maxhidapi * x, long length, long report_id);
void maxhidapi_info(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv);

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
    class_addmethod(c, (method)maxhidapi_info, "info", A_GIMME, 0);
    class_register(CLASS_BOX, c);
    s_maxhidapi_class = c;
}

void maxhidapi_open(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    if(x->device != NULL){
        maxhidapi_close(x);
    }
    // 1 argument - open using index number
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
                x->device = hid_open_path(cur_dev->path);
                x->vendor_id = cur_dev->vendor_id;
                x->product_id = cur_dev->product_id;
                break;
            }
            cur_dev = cur_dev->next;
            ++i;
        }
    }
    // 2 argument - open using vendor id and product id
    if(argc == 2){
        x->vendor_id = (unsigned short)atom_getlong(argv);
        x->product_id = (unsigned short)atom_getlong(argv + 1);
        x->device = hid_open(x->vendor_id, x->product_id, NULL);
    }
    // 3 argument - open using vendor id, product id and serial number
    else if(argc == 3){
        x->vendor_id = (unsigned short)atom_getlong(argv);
        x->product_id = (unsigned short)atom_getlong(argv + 1);
        char * serial_number_c = atom_getsym(argv + 2)->s_name;
        size_t length = strlen(serial_number_c);
        wchar_t * serial_number_w = (wchar_t *)malloc((length + 1) * sizeof(wchar_t));
        mbstowcs(serial_number_w, serial_number_c, length);
        *(serial_number_w + length) = L'\0';
        x->device = hid_open(x->vendor_id, x->product_id, serial_number_w);
        free(serial_number_w);
    }
    // if device not opened
    if(x->device == NULL){
        t_atom status;
        atom_setlong(&status, -1);
        outlet_anything(x->outlet, gensym("open"), 1, &status);
    }
    // if succesfully opened
    else{
        maxhidapi_set_nonblocking(x, 1);
        t_atom status;
        atom_setlong(&status, 0);
        outlet_anything(x->outlet, gensym("open"), 1, &status);
    }
}

void maxhidapi_write(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    if(x->device != NULL){
        int i;
        unsigned char length = (unsigned char)atom_getlong(argv);
        unsigned char * buffer = (unsigned char *)calloc(length, sizeof(unsigned char));
        if(buffer == NULL){
            t_atom respond;
            atom_setlong(&respond, -2);
            outlet_anything(x->outlet, gensym("write"), 1, &respond);
            return;
        }
        for(i = 0; i < argc - 1; i++){
            * (buffer + i) = (unsigned char)atom_getlong(argv + 1 + i);
        }
        int result = hid_write(x->device, buffer, length);
        free(buffer);
        t_atom respond;
        atom_setlong(&respond, result);
        outlet_anything(x->outlet, gensym("write"), 1, &respond);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_read(t_maxhidapi * x, long length, long report_id){
    if(x->device != NULL){
        unsigned char * buffer = (unsigned char *)malloc(length * sizeof(unsigned char));
        if(report_id >= 0){
            * (buffer) = report_id;
        }
        int result = hid_read(x->device, buffer, length);
        if(result >= 0){
            t_atom * read_buffer = (t_atom *)malloc(length * sizeof(t_atom));
            int i;
            for(i = 0; i < length; i++){
                atom_setlong(read_buffer + i, *(buffer + i));
            }
            outlet_anything(x->outlet, gensym("read"), result, read_buffer);
            free(read_buffer);
        }
        else{
            t_atom respond;
            atom_setlong(&respond, result);
            outlet_anything(x->outlet, gensym("read"), 1, &respond);
        }
        free(buffer);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_close(t_maxhidapi * x){
    if(x->device != NULL){
        hid_close(x->device);
        x->device = NULL;
        x->vendor_id = 0;
        x->product_id = 0;
        t_atom respond;
        atom_setlong(&respond, 0);
        outlet_anything(x->outlet, gensym("close"), 1, &respond);
    }
}

void maxhidapi_get_product_string(t_maxhidapi * x){
    if(x->device != NULL){
        wchar_t * product_string_w = (wchar_t *)malloc(256 * sizeof(wchar_t));
        int result = hid_get_product_string(x->device, product_string_w, 256);
        size_t length = wcslen(product_string_w);
        char * product_string_c = (char *)malloc((length + 1) * sizeof(char));
        if(result == 0){
            wcstombs(product_string_c, product_string_w, length);
            *(product_string_c + length) = '\0';
            t_atom product_string_a;
            atom_setsym(&product_string_a, gensym(product_string_c));
            outlet_anything(x->outlet, gensym("get_product_string"), 1, &product_string_a);
        }
        else{
            t_atom respond;
            atom_setlong(&respond, result);
            outlet_anything(x->outlet, gensym("get_product_string"), 1, &respond);
        }
        free(product_string_c);
        free(product_string_w);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_manufacturer_string(t_maxhidapi * x){
    if(x->device != NULL){
        wchar_t * manufacturer_string_w = (wchar_t *)malloc(256 * sizeof(wchar_t));
        int result = hid_get_manufacturer_string(x->device, manufacturer_string_w, 256);
        size_t length = wcslen(manufacturer_string_w);
        char * manufacturer_string_c = (char *)malloc((length + 1) * sizeof(char));
        if(result == 0){
            wcstombs(manufacturer_string_c, manufacturer_string_w, length);
            *(manufacturer_string_c + length) = '\0';
            t_atom manufacturer_string_a;
            atom_setsym(&manufacturer_string_a, gensym(manufacturer_string_c));
            outlet_anything(x->outlet, gensym("get_manufacturer_string"), 1, &manufacturer_string_a);
        }
        else{
            t_atom respond;
            atom_setlong(&respond, result);
            outlet_anything(x->outlet, gensym("get_manufacturer_string"), 1, &respond);
        }
        free(manufacturer_string_c);
        free(manufacturer_string_w);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_serial_number_string(t_maxhidapi * x){
    if(x->device != NULL){
        wchar_t * serial_number_string_w = (wchar_t *)malloc(256 * sizeof(wchar_t));
        int result = hid_get_serial_number_string(x->device, serial_number_string_w, 256);
        size_t length = wcslen(serial_number_string_w);
        char * serial_number_string_c = (char *)malloc((length + 1) * sizeof(char));
        if(result == 0){
            wcstombs(serial_number_string_c, serial_number_string_w, length);
            *(serial_number_string_c + length) = '\0';
            t_atom serial_number_string_a;
            atom_setsym(&serial_number_string_a, gensym(serial_number_string_c));
            outlet_anything(x->outlet, gensym("get_serial_number_string"), 1, &serial_number_string_a);
        }
        else{
            t_atom respond;
            atom_setlong(&respond, result);
            outlet_anything(x->outlet, gensym("get_serial_number_string"), 1, &respond);
        }
        free(serial_number_string_c);
        free(serial_number_string_w);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_indexed_string(t_maxhidapi * x, long index){
    if(x->device != NULL){
        wchar_t * indexed_string_w = (wchar_t *)malloc(256 * sizeof(wchar_t));
        int result = hid_get_indexed_string(x->device, index, indexed_string_w, 256);
        size_t length = wcslen(indexed_string_w);
        char * indexed_string_c = (char *)malloc((length + 1) * sizeof(char));
        if(result == 0){
            wcstombs(indexed_string_c, indexed_string_w, length);
            *(indexed_string_c + length) = '\0';
            t_atom indexed_string_a;
            atom_setsym(&indexed_string_a, gensym(indexed_string_c));
            outlet_anything(x->outlet, gensym("get_indexed_string"), 1, &indexed_string_a);
        }
        else{
            t_atom respond;
            atom_setlong(&respond, result);
            outlet_anything(x->outlet, gensym("get_indexed_string"), 1, &respond);
        }
        free(indexed_string_c);
        free(indexed_string_w);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_error(t_maxhidapi * x){
    if(x->device != NULL){
        const wchar_t * error_w = hid_error(x->device);
        if(error_w == NULL){
            t_atom respond;
            atom_setlong(&respond, -1);
            outlet_anything(x->outlet, gensym("error"), 1, &respond);
            return;
        }
        int length = wcslen(error_w);
        char * error_c = (char *)malloc((length + 1) * sizeof(char));
        wcstombs(error_c, error_w, length);
        *(error_c + length) = '\0';
        t_atom error_a;
        atom_setsym(&error_a, gensym(error_c));
        outlet_anything(x->outlet, gensym("error"), 1, &error_a);
        free(error_c);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_set_nonblocking(t_maxhidapi * x, long nonblock){
    if(x->device != NULL){
        int result = hid_set_nonblocking(x->device, nonblock);
        t_atom respond;
        atom_setlong(&respond, result);
        outlet_anything(x->outlet, gensym("set_nonblocking"), 1, &respond);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_enumerate(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    if(x->devices != NULL){
        hid_free_enumeration(x->devices);
        x->devices = NULL;
    }
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
    t_atom * respond = (t_atom *)malloc(11 * sizeof(t_atom));
    char * str_output;
    str_output = (char *)malloc(1);
    wchar_t * str_input;
    size_t length;
    while(cur_dev){
        // 1 index
        atom_setlong(respond, i);
        // 2 vendor id
        atom_setlong(respond + 1, cur_dev->vendor_id);
        // 3 product id
        atom_setlong(respond + 2, cur_dev->product_id);
        // 4 serial number
        str_input = cur_dev->serial_number;
        if(str_input == NULL){
            str_output = (char *)realloc(str_output, 5 * sizeof(char));
            strcpy(str_output, "null");
        }
        else{
            length = wcslen(str_input);
            str_output = (char *)realloc(str_output, (length + 1) * sizeof(char));
            wcstombs(str_output, str_input, length);
            *(str_output + length) = '\0';
        }
        atom_setsym(respond + 3, gensym(str_output));
        // 5 manufacturer name
        str_input = cur_dev->manufacturer_string;
        if(str_input == NULL){
            str_output = (char *)realloc(str_output, 5 * sizeof(char));
            strcpy(str_output, "null");
        }
        else{
            length = wcslen(str_input);
            str_output = (char *)realloc(str_output, (length + 1) * sizeof(char));
            wcstombs(str_output, str_input, length);
            *(str_output + length) = '\0';
        }
        atom_setsym(respond + 4, gensym(str_output));
        // 6 product name
        str_input = cur_dev->product_string;
        if(str_input == NULL){
            str_output = (char *)realloc(str_output, 5 * sizeof(char));
            strcpy(str_output, "null");
        }
        else{
            length = wcslen(str_input);
            str_output = (char *)realloc(str_output, (length + 1) * sizeof(char));
            wcstombs(str_output, str_input, length);
            *(str_output + length) = '\0';
        }
        atom_setsym(respond + 5, gensym(str_output));
        // 7 release number
        atom_setlong(respond + 6, cur_dev->release_number);
        // 8 interface number
        atom_setlong(respond + 7, cur_dev->interface_number);
        // 9 usage
        atom_setlong(respond + 8, cur_dev->usage);
        // 10 usage page
        atom_setlong(respond + 9, cur_dev->usage_page);
        // 11 path
        atom_setsym(respond + 10, gensym(cur_dev->path));
        // send to the outlet
        outlet_anything(x->outlet, gensym("enumerate"), 11, respond);
        cur_dev = cur_dev->next;
        ++i;
    }
    free(str_output);
    free(respond);
}

void maxhidapi_open_path(t_maxhidapi * x, t_symbol * s){
    if(x->device != NULL){
        maxhidapi_close(x);
    }
    hid_device * device;
    const char * path = s->s_name;
    device = hid_open_path(path);
    if(device == NULL){
        t_atom respond;
        atom_setlong(&respond, -1);
        outlet_anything(x->outlet, gensym("open_path"), 1, &respond);
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
            t_atom respond;
            atom_setlong(&respond, -1);
            outlet_anything(x->outlet, gensym("open_path"), 1, &respond);
        }
        else{
            maxhidapi_set_nonblocking(x, 1);
            t_atom respond;
            atom_setlong(&respond, 0);
            outlet_anything(x->outlet, gensym("open_path"), 3, &respond);
        }
    }
}

void maxhidapi_send_feature_report(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    if(x->device != NULL){
        int i;
        unsigned char length = (unsigned char)atom_getlong(argv);
        unsigned char * buffer = (unsigned char *)calloc(length, sizeof(unsigned char));
        for(i = 0; i < argc - 1; i++){
            *(buffer + i) = (unsigned char)atom_getlong(argv + 1 + i);
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

void maxhidapi_get_feature_report(t_maxhidapi * x, long length, long report_id){
    if(x->device != NULL){
        unsigned char * buffer = (unsigned char *)malloc(length * sizeof(unsigned char));
        *(buffer) = report_id;
        int result = hid_get_feature_report(x->device, buffer, length);
        if(result >= 0){
            t_atom * respond = (t_atom *)malloc(result * sizeof(t_atom));
            int i;
            for(i = 0; i < result; i++){
                atom_setlong(respond + i, *(buffer + i));
            }
            outlet_anything(x->outlet, gensym("get_feature_report"), result, respond);
            free(respond);
        }
        else{
            t_atom respond;
            atom_setlong(&respond, result);
            outlet_anything(x->outlet, gensym("get_feature_report"), 1, &respond);
        }
        free(buffer);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_get_input_report(t_maxhidapi * x, long length, long report_id){
    if(x->device != NULL){
        unsigned char * buffer = (unsigned char *)malloc(length * sizeof(unsigned char));
        *(buffer) = report_id;
        int result = hid_get_input_report(x->device, buffer, length);
        if(result >= 0){
            t_atom * respond = (t_atom *)malloc(result * sizeof(t_atom));
            int i;
            for(i = 0; i < result; i++){
                atom_setlong(respond + i, *(buffer + i));
            }
            outlet_anything(x->outlet, gensym("get_input_report"), result, respond);
            free(respond);
        }
        else{
            t_atom respond;
            atom_setlong(&respond, result);
            outlet_anything(x->outlet, gensym("get_input_report"), 1, &respond);
        }
        free(buffer);
    }
    else{
        post("Not connected");
    }
}

void maxhidapi_info(t_maxhidapi * x, t_symbol * s, long argc, t_atom * argv){
    unsigned short vendor_id = 0, product_id = 0;
    if (argc == 1){
        vendor_id = (unsigned short)atom_getlong(argv);
    }
    else if (argc == 2){
        vendor_id = (unsigned short)atom_getlong(argv);
        product_id = (unsigned short)atom_getlong(argv + 1);
    }
    struct hid_device_info * devices, * cur_dev;
    devices = hid_enumerate(vendor_id, product_id);
    cur_dev = devices;
    int i = 1;
    while(cur_dev){
        post("%d  id: %04hx %04hx  serial_number: %ls", i, cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);
        post("  Manufacturer: %ls", cur_dev->manufacturer_string);
        post("  Product:      %ls", cur_dev->product_string);
        post("  Release:      %hx", cur_dev->release_number);
        post("  Interface:    %d",  cur_dev->interface_number);
        post("  Usage (page): 0x%hx (0x%hx)", cur_dev->usage, cur_dev->usage_page);
        post("  Path: %s", cur_dev->path);
        post(" ");
        cur_dev = cur_dev->next;
        ++i;
    }
    hid_free_enumeration(devices);
}