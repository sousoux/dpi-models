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

#ifndef __DPI_MODELS_HPP__
#define __DPI_MODELS_HPP__

#include <json.hpp>
#include <map>

#ifdef USE_DPI
#include "questa/dpiheader.h"
#endif



class Dpi_itf
{
  public:
    void bind(void *handle);

  protected:
    void *sv_handle;
};



class Qspi_itf : public Dpi_itf
{
  public:
    virtual void sck_edge(int64_t timestamp, int sck, int data_0, int data_1, int data_2, int data_3) {};
    void set_data(int data_0);
    void set_qpi_data(int data_0, int data_1, int data_2, int data_3);
};



class Jtag_itf : public Dpi_itf
{
  public:
    void tck_edge(int tck, int tdi, int tms, int trst, int *tdo);
};



class Uart_itf : public Dpi_itf
{
  public:
    virtual void tx_edge(int64_t timestamp, int data) {}
    void rx_edge(int data);
};



class Cpi_itf : public Dpi_itf
{
  public:
    virtual void edge(int64_t timestamp, int pclk, int hsync, int vref, int data) {}
    void edge(int pclk, int hsync, int vref, int data);
};




class Ctrl_itf : public Dpi_itf
{
  public:
    void reset_edge(int reset);
};




class Dpi_model
{
public:
  Dpi_model(js::config *config, void *handle);
  void *bind_itf(std::string name, void *handle);
  void create_itf(std::string name, Dpi_itf *itf);
  void create_task(void *arg1, void *arg2);
  void create_periodic_handler(int64_t period, void *arg1, void *arg2);
  void wait(int64_t ns);
  void wait_ps(int64_t ps);
  void wait_event();
  void raise_event();
  void raise_event_from_ext();
  virtual void start() {};

protected:
  void print(const char *format, ...);
  js::config *get_config();

private:
  js::config *config;
  std::map<std::string, Dpi_itf *> itfs;
  void *handle;
};



#endif