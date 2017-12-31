#include <iostream>
#include <stdexcept>
#include <vector>
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <GLUtil.hpp>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include <nuklear/nuklear.h>
#include <nuklear/demo/glfw_opengl3/nuklear_glfw_gl3.h>
#include <nuklear/demo/style.c>

#include <gmp.h>
#include <gmpxx.h>

#include <StringView.hpp>
#include <Coding.hpp>

#define PRAC2 "Error Correction"

#define MAX_VERTEX_BUFFER (512 * 1024)
#define MAX_ELEMENT_BUFFER (128 * 1024)


class Window {
  GLFWwindow *win = nullptr;
  int width_,height_;
  struct nk_context *ctx=nullptr;

public:
  Window(int width, int height):
    width_(width), height_(height)
  {}

  int width() const {
    return width_;
  }

  int height() const {
    return height_;
  }

  ~Window() {
  }
private:
  void setup_glfw() {
    GLFW::init_glfw3();
    win = GLFW::new_window(width_, height_, PRAC2);
    GLFW::get_win_size(win, width_, height_);

    glViewport(0, 0, width(), height());
    GLEW::init_glew();
  }

  void setup_nk() {
    setup_glfw();
    ctx = nk_glfw3_init(win, NK_GLFW3_INSTALL_CALLBACKS);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    {
      struct nk_font_atlas *atlas;
      nk_glfw3_font_stash_begin(&atlas);
      struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/DroidSans.ttf", 24, 0);
      /*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Roboto-Regular.ttf", 14, 0);*/
      /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/kenvector_future_thin.ttf", 13, 0);*/
      /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyClean.ttf", 12, 0);*/
      /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/ProggyTiny.ttf", 10, 0);*/
      /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "nuklear/extra_font/Cousine-Regular.ttf", 13, 0);*/
      nk_glfw3_font_stash_end();
      /* nk_style_load_all_cursors(ctx, atlas->cursors); */
      nk_style_set_font(ctx, &droid->handle);
    }
    background = nk_rgb(136,181,216);
  }

  int_t len = 7;
  int_t r = 2; // parameter for hamming code
  int_t e = 1; // parameter for repitition code
  int_t k = 3; // parameter for repitition code
  float probability = 0.005;
  const double increase = 1.5;
  bitset_t msg, enc, rec, dec;
  struct nk_color background;

  void clear_message() {
    msg.clear();
  }

  void generate_message() {
    int_t old_len = msg.size();
    msg.resize(len);
    if(old_len < len) {
      msg.set_random(old_len, len);
    }
  }

  enum { HAMMING, REPITITION, HADAMARD };
  int_t min_dist(int coderOption) {
    if(coderOption == HAMMING) {
      return HammingCode::min_dist(r);
    } else if(coderOption == REPITITION) {
      return RepititionCode::min_dist(e);
    } else if(coderOption == HADAMARD) {
      return HadamardCode::min_dist(k);
    }
    return -1;
  }
  int_t get_source_dim(int coderOption) {
    if(coderOption == HAMMING) {
      return HammingCode::get_source_dim(r);
    } else if(coderOption == REPITITION) {
      return RepititionCode::get_source_dim(e);
    } else if(coderOption == HADAMARD) {
      return HadamardCode::get_source_dim(k);
    }
    return -1;
  }
  int_t get_dest_dim(int coderOption) {
    if(coderOption == HAMMING) {
      return HammingCode::get_dest_dim(r);
    } else if(coderOption == REPITITION) {
      return RepititionCode::get_dest_dim(e);
    } else if(coderOption == HADAMARD) {
      return HadamardCode::get_dest_dim(k);
    }
    return -1;
  }

  int_t err = 0;
  template <typename T>
  Bitset<T> transmit(const Bitset<T> &v) {
    /* printf("GENERATING ERRORS\n"); */
    Bitset<T> bset = v;
    err = bset.set_errors(probability);
    return bset;
  }

  template <typename CoderT, typename... As>
  void process_message(As... args) {
    CoderT coder(args...);
    dec=coder.decode(rec=transmit(enc=coder.encode(msg)));
  }


  void draw_label(std::string s, int h=30, int pos=NK_TEXT_CENTERED) {
    draw_label(s.c_str(), h, pos);
  }

  void draw_label(const char *labelName, int h=30, int pos=NK_TEXT_CENTERED) {
    nk_layout_row_dynamic(ctx, h, 1);
    nk_label(ctx, labelName, pos);
  }

  void draw_space(int h=20) {
    nk_layout_row_dynamic(ctx, h, 0);
  }

private:
  mpf_class c_nk(int n, int k) {
    mpf_class n_k = 1;
    for(int i = n - k + 1; i <= n; ++i) {
      n_k *= i;
    }
    mpz_class kfac;
    mpz_fac_ui(kfac.get_mpz_t(), k);
    /* std::cout << "binomial coeff: " << n_k << "/ " << kfac << std::endl; */
    return n_k / kfac;
  }

  mpf_class exp(double x, int_t p) {
    mpf_class t = x;
    mpf_pow_ui(t.get_mpf_t(), t.get_mpf_t(), p);
    /* printf("exp == %f\n", t.get_d()); */
    return t;
  }

  mpf_class get_binomial(double p, int_t k, int_t e) {
    return (exp(1. - p, k - e) * exp(p, e) * c_nk(k, e));
  }

  float prob_of_correction(double p, int_t k, int_t e) {
    double sum = 0;
    for(int i = 0; i <= e; ++i) {
      sum += get_binomial(p, k, i).get_d();
    }
    return sum;
  }

  int_t errors_corrected(int coderOption) {
    int_t errors = -1;
    if(coderOption == HAMMING) errors = 1;
    if(coderOption == REPITITION) errors = e;
    if(coderOption == HADAMARD) errors = ((1 << (k - 1)) - 1) >> 1;
    return errors;
  }

  void update_prob_of_correction(int coderOption) {
    p_correction = prob_of_correction(probability, get_dest_dim(coderOption), errors_corrected(coderOption));
  }
public:

  float p_correction = -1;

  void idle() {
    /* Input */
    glfwPollEvents();
    nk_glfw3_new_frame();

    /* GUI */
    if (nk_begin(ctx, PRAC2, nk_rect(25, 25, width()-50, height()-50),
          NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
          NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
      static int coderOption = HAMMING;
      if(p_correction < 0) {
        update_prob_of_correction(coderOption);
      }
      nk_layout_row_dynamic(ctx, 150, 2);
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        draw_label("Transmission", 30, NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 30, 2);
        nk_label(ctx, ("p=" + std::to_string(probability)).c_str(), NK_TEXT_LEFT);
        float last_prob = probability;
        nk_slider_float(ctx, 0, &probability, 0.499999f, 1e-2 * (probability + 1e-2));
        if(last_prob != probability) {
          update_prob_of_correction(coderOption);
        }
        nk_group_end(ctx);
      }
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        draw_label("Algorithm");
        nk_layout_row_dynamic(ctx, 30, 3);
        int last_coder_opt = coderOption;
        if (nk_option_label(ctx, "Hamming Code", coderOption == HAMMING)) coderOption = HAMMING;
        if (nk_option_label(ctx, "Repitition Code", coderOption == REPITITION)) coderOption = REPITITION;
        if (nk_option_label(ctx, "Hadamard Code", coderOption == HADAMARD)) coderOption = HADAMARD;
        if(last_coder_opt != coderOption) {
          update_prob_of_correction(coderOption);
        }

        if(coderOption == HAMMING) {
          nk_layout_row_dynamic(ctx, 50, 8);
          int prev_r = r;
          for(int ir = 2; ir <= 9; ++ir) {
            if(nk_option_label(ctx, ("R == " + std::to_string(ir)).c_str(), r == ir)) r = ir;
          }
          if(r != prev_r) update_prob_of_correction(coderOption);
        } else if(coderOption == REPITITION) {
          nk_layout_row_dynamic(ctx, 50, 8);
          int prev_e = e;
          for(int ie = 1; ie <= 8; ++ie) {
            if(nk_option_label(ctx, ("E == " + std::to_string(ie)).c_str(), e == ie)) e = ie;
          }
          if(e != prev_e) update_prob_of_correction(coderOption);
        } else if(coderOption == HADAMARD) {
          nk_layout_row_dynamic(ctx, 50, 8);
          int prev_k = k;
          for(int ik = 3; ik <= 10; ++ik) {
            if(nk_option_label(ctx, ("K == " + std::to_string(ik)).c_str(), k == ik)) k = ik;
          }
          if(k != prev_k) update_prob_of_correction(coderOption);
        }
        nk_group_end(ctx);
      }

      nk_layout_row_dynamic(ctx, 150, 2);
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        draw_label("Input (" + std::to_string(len) + ")");
        nk_layout_row_dynamic(ctx, 30, 1);
        TextBox<BitsetStringView> txtboxmsg(BitsetStringView(msg), 50, 1);
        static float scroll_input = 0.;
        nk_slider_float(ctx, 0, &scroll_input, 1.f, 1./txtboxmsg.no_lines());
        txtboxmsg.set_pos_f(scroll_input);
        nk_layout_row_dynamic(ctx, 50, 1);
        nk_flags input_event = nk_edit_string_zero_terminated(ctx, 0, (char*)txtboxmsg.lines.c_str(), txtboxmsg.size(), nk_filter_binary);
        nk_group_end(ctx);
      }
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 30, 3);
        nk_label(ctx, "N = ", NK_TEXT_LEFT);
        static char num[80] = "7";
        nk_flags n_change_event = nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX | NK_EDIT_AUTO_SELECT, num, 80, nk_filter_decimal);
        len = std::abs(atoi(num));
        generate_message();
        nk_layout_row_dynamic(ctx, 30, 6);
        if(nk_button_label(ctx, "Go")) {
          clear_message();
          generate_message();
          switch(coderOption) {
            case HAMMING: process_message<HammingCode>(r); break;
            case REPITITION: process_message<RepititionCode>(e); break;
            case HADAMARD: process_message<HadamardCode>(k); break;
          }
        }
        nk_group_end(ctx);
      }

      nk_layout_row_dynamic(ctx, 150, 1);
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        std::string encoded_label = "Encoded (" + std::to_string(enc.size()) + ")";
        switch(coderOption) {
          case HAMMING:
            encoded_label += " [r=" + std::to_string(r)
              + ", k=" + std::to_string(HammingCode::get_source_dim(r))
              + ", n=" + std::to_string(HammingCode::get_dest_dim(r)) + "]";
          break;
          case REPITITION:
            encoded_label += " [e=" + std::to_string(e)
              + ", d=" + std::to_string(2*e+1) + "]";
          break;
          case HADAMARD:
            encoded_label += " [k=" + std::to_string(k)
              + ", n=" + std::to_string(HadamardCode::get_dest_dim(k))+ "]";
          break;
        }
        draw_label(encoded_label);
        nk_layout_row_dynamic(ctx, 30, 1);
        TextBox<BitsetStringView> txtboxenc(BitsetStringView(enc), 100, 1);
        static float scroll_encoded = 0.;
        nk_slider_float(ctx, 0, &scroll_encoded, 1.f, 1./txtboxenc.no_lines());
        txtboxenc.set_pos_f(scroll_encoded);
        nk_layout_row_dynamic(ctx, 50, 1);
        nk_flags encoded_event = nk_edit_string_zero_terminated(ctx, 0, (char*)txtboxenc.lines.c_str(), txtboxenc.size(), nk_filter_binary);
        nk_group_end(ctx);
      }

      nk_layout_row_dynamic(ctx, 150, 1);
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        draw_label("Received (" + std::to_string(rec.size()) + ") [e = " + std::to_string(err) + "]");
        nk_layout_row_dynamic(ctx, 30, 1);
        TextBox<BitsetStringView> txtboxrec(BitsetStringView(rec), 100, 1);
        static float scroll_received = 0.;
        nk_slider_float(ctx, 0, &scroll_received, 1.f, 1.f/txtboxrec.no_lines());
        txtboxrec.set_pos_f(scroll_received);
        nk_layout_row_dynamic(ctx, 50, 1);
        nk_flags received_event = nk_edit_string_zero_terminated(ctx, 0, (char*)txtboxrec.lines.c_str(), txtboxrec.size(), nk_filter_binary);
        nk_group_end(ctx);
      }

      nk_layout_row_dynamic(ctx, 250, 2);
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        draw_label("Decoded (" + std::to_string(dec.size()) + ")");
        nk_layout_row_dynamic(ctx, 30, 1);
        TextBox<BitsetStringView> txtboxdec(BitsetStringView(dec), 50, 1);
        static float scroll_decoded = 0.;
        nk_slider_float(ctx, 0, &scroll_decoded, 1.f, 1.f/txtboxdec.no_lines());
        txtboxdec.set_pos_f(scroll_decoded);
        nk_layout_row_dynamic(ctx, 50, 1);
        nk_flags decoded_event = nk_edit_string_zero_terminated(ctx, 0, (char*)txtboxdec.lines.c_str(), txtboxdec.size(), nk_filter_binary);
        nk_group_end(ctx);
      }
      if(nk_group_begin(ctx, "", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 50, 3);
        int_t hammingdist = bitset_t::hamming_distance(msg, dec);
        nk_label(ctx, "Hamming distance: ", NK_TEXT_LEFT);
        nk_label(ctx, std::to_string(hammingdist).c_str(), NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 50, 3);
        nk_label(ctx, "Information Rate: ", NK_TEXT_LEFT);
        nk_label(ctx, std::to_string(float(get_source_dim(coderOption)) / float(get_dest_dim(coderOption))).c_str(), NK_TEXT_LEFT);
        draw_label("Probability of correction:");
        nk_layout_row_dynamic(ctx, 50, 3);
        nk_label(ctx, ("P(e <= " + std::to_string(errors_corrected(coderOption)) + "): ").c_str(), NK_TEXT_LEFT);
        nk_label(ctx, std::to_string(p_correction).c_str(), NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 50, 3);
        nk_label(ctx, "Corrected errors: ", NK_TEXT_LEFT);
        float rat = float(err ? float(err-hammingdist) / float(err) : 1);
        nk_label(ctx, std::to_string(rat).c_str(), NK_TEXT_LEFT);
        nk_group_end(ctx);
      }

      draw_label("Background");
      nk_layout_row_dynamic(ctx, 25, 1);
      if (nk_combo_begin_color(ctx, background, nk_vec2(nk_widget_width(ctx),400))) {
        nk_layout_row_dynamic(ctx, 120, 1);
        background = nk_color_picker(ctx, background, NK_RGBA);
        nk_layout_row_dynamic(ctx, 25, 1);
        background.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, background.r, 255, 1,1);
        background.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, background.g, 255, 1,1);
        background.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, background.b, 255, 1,1);
        background.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, background.a, 255, 1,1);
        nk_combo_end(ctx);
      }
    }
    nk_end(ctx);
    /* set_style(ctx, THEME_WHITE); */
    set_style(ctx, THEME_RED);
    /* set_style(ctx, THEME_BLUE); */
    /* set_style(ctx, THEME_DARK); */

    /* Draw */
    {
      float bg[4];
      nk_color_fv(bg, background);
      glfwGetWindowSize(win, &width_, &height_);
      glViewport(0, 0, width_, height_);
      glClear(GL_COLOR_BUFFER_BIT);
      glClearColor(bg[0], bg[1], bg[2], bg[3]);
      /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
       * with blending, scissor, face culling, depth test and viewport and
       * defaults everything back into a default state.
       * Make sure to either a.) save and restore or b.) reset your own state after
       * rendering the UI. */
      nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
      glfwSwapBuffers(win);
    }
  }

  void terminate() {
    nk_glfw3_shutdown();
    glfwTerminate();
  }
public:
  void run() {
    setup_nk();
    while(!glfwWindowShouldClose(win)) {
      idle();
    }
    terminate();
  }
};

int main() {
  srand(time(nullptr));
  Window w(1440,1080);
  w.run();
}
