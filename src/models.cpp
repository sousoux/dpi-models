/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>

#include <json.hpp>
#include <dlfcn.h>

#include "dpi/models.hpp"

static std::vector<void *> tasks_cb;
static std::vector<void *> tasks_arg;
static std::vector<void *> handlers_cb;
static std::vector<void *> handlers_arg;

static void dpi_fatal_stub(void *handle, const char *format, ...)
{
  int size = 1024;
  while(1)
  {
    char str[size];
    va_list ap;
    va_start(ap, format);
    int iter_size = vsnprintf(str, size, format, ap);
    va_end(ap);
    if (iter_size <= size)
    {
      dpi_fatal(handle, str);
      break;
    }
    size = iter_size;
  }
}

static void dpi_print_stub(void *handle, const char *format, ...)
{
  int size = 1024;
  while(1)
  {
    char str[size];
    va_list ap;
    va_start(ap, format);
    int iter_size = vsnprintf(str, size, format, ap);
    va_end(ap);
    if (iter_size <= size)
    {
      dpi_print(handle, str);
      break;
    }
    size = iter_size;
  }
}

void Dpi_model::print(const char *format, ...)
{
  int size = 1024;
  while(1)
  {
    char str[size];
    va_list ap;
    va_start(ap, format);
    int iter_size = vsnprintf(str, size, format, ap);
    va_end(ap);
    if (iter_size <= size)
    {
      dpi_print(handle, str);
      break;
    }
    size = iter_size;
  }
}

void Dpi_model::create_task(void *arg1, void *arg2)
{
  int task_id = tasks_cb.size();
  tasks_cb.push_back(arg1);
  tasks_arg.push_back(arg2);
  dpi_create_task(handle, task_id);
}

void Dpi_model::create_periodic_handler(int64_t period, void *arg1, void *arg2)
{
  int handler_id = handlers_cb.size();
  handlers_cb.push_back(arg1);
  handlers_arg.push_back(arg2);
  dpi_create_periodic_handler(handle, handler_id, period);
}

int dpi_start_task(int id)
{
  ((void (*)(void *))tasks_cb[id])(tasks_arg[id]);
  return 0;
}

void dpi_exec_periodic_handler(int id)
{
  ((void (*)(void *))handlers_cb[id])(handlers_arg[id]);
}

Dpi_model::Dpi_model(js::config *config, void *handle) : config(config), handle(handle)
{

}

void Dpi_model::wait(int64_t ns)
{
  dpi_wait(handle, ns);
}

void Dpi_model::wait_ps(int64_t ps)
{
  dpi_wait_ps(handle, ps);
}

void Dpi_model::wait_event()
{
  dpi_wait_event(handle);
}

// This function is only useful on virtual platform to avoid active polling
// between the a task and a thread
void Dpi_model::raise_event()
{
  // TODO
  // Dirty hack to not call system verilog task from pthread on RTL platform
  if (handle)
    dpi_raise_event(handle);
}

void Dpi_model::raise_event_from_ext()
{
  // TODO
  // Dirty hack to not call system verilog task from pthread on RTL platform
  if (handle)
    dpi_raise_event_from_ext(handle);
}


void Dpi_itf::bind(void *handle)
{
  sv_handle = handle;
}

void *Dpi_model::bind_itf(std::string name, void *handle)
{
  if (itfs.find(name) == itfs.end())
    return NULL;
  itfs[name]->bind(handle);
  return (void *)itfs[name];
}

void Dpi_model::create_itf(std::string name, Dpi_itf *itf)
{
  itfs[name] = itf;
}

js::config *Dpi_model::get_config()
{
  return config;
}

extern "C" void *model_load(void *_config, void *handle)
{
  js::config *config = (js::config *)_config;
  js::config *module_config = config->get("module");
  if (module_config == NULL)
  {
    dpi_fatal_stub(handle, "ERROR, Failed to open periph model, didn't find configuration item 'model'");
    return NULL; 
  }


  const char *module_name = module_config->get_str().c_str();

  void *module = dlopen(module_name, RTLD_NOW | RTLD_GLOBAL | RTLD_DEEPBIND);
  if (module == NULL)
  {
    dpi_fatal_stub(handle, "ERROR, Failed to open periph model (%s) with error: %s", module_name, dlerror());
    return NULL;
  }

  Dpi_model *(*model_new)(js::config *, void *) = (Dpi_model *(*)(js::config *, void *))dlsym(module, "dpi_model_new");
  if (model_new == NULL)
  {
    dpi_fatal_stub(handle, "ERROR, invalid DPI model being loaded (%s)", module_name);
    return NULL;
  }

  return model_new(config, handle);
}
