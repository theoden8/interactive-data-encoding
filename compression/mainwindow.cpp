#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>

#include <coding/Coding.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->radioNoCoding->click();
  ui->doubleSpinBox_1->setVisible(false);
  ui->doubleSpinBox_2->setVisible(false);
  ui->doubleSpinBox_3->setVisible(false);
  ui->doubleSpinBox_4->setVisible(false);
  ui->doubleSpinBox_5->setVisible(false);
  ui->doubleSpinBox_6->setVisible(false);
  ui->doubleSpinBox_7->setVisible(false);
  ui->doubleSpinBox_8->setVisible(false);
  ui->doubleSpinBox_EOT->setVisible(false);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::update_alphabet_text() {
  on_textAlphabet_textChanged();
}

void MainWindow::update_input_text() {
  on_textInput_textChanged();
}

// generate random string depending on probabilities
std::string genstring(const std::string &symbols, const std::vector<float> &probs, int len) {
  if(symbols.length() == 0) {
    return std::string();
  }
  std::vector<int> counts(probs.size(), 0);
  for(int i = 0; i < probs.size(); ++i) {
    counts[i] = probs[i] * 10000;
    if(i) {
      counts[i] += counts[i - 1];
    }
  }
  char result[len + 1];
  result[len] = '\0';
  for(int i = 0; i < len; ++i) {
    int pick = rand() % counts.back();
    for(int j = 0; j < counts.size(); ++j) {
      if(pick <= counts[j]) {
        result[i] = symbols[j];
        break;
      }
    }
  }
  return std::string(result);
}

// generate input message
void MainWindow::on_btnGen_clicked() {
  auto alphabet = ui->textAlphabet->toPlainText().toStdString();
  ui->btnAdjust->click();
  auto len = alphabet.length();
  std::vector<float> probs;
  if(len >= 1) probs.push_back(ui->doubleSpinBox_1->value());
  if(len >= 2) probs.push_back(ui->doubleSpinBox_2->value());
  if(len >= 3) probs.push_back(ui->doubleSpinBox_3->value());
  if(len >= 4) probs.push_back(ui->doubleSpinBox_4->value());
  if(len >= 5) probs.push_back(ui->doubleSpinBox_5->value());
  if(len >= 6) probs.push_back(ui->doubleSpinBox_6->value());
  if(len >= 7) probs.push_back(ui->doubleSpinBox_7->value());
  if(len >= 8) probs.push_back(ui->doubleSpinBox_8->value());
  ui->textInput->document()->setPlainText(QString::fromStdString(genstring(alphabet, probs, rand() % 1000)));
}

void MainWindow::on_textAlphabet_textChanged() {
  auto s = ui->textAlphabet->toPlainText().toStdString();
  std::string s2;
  s2.reserve(s.length());
  std::vector<bool> seen(256, false);
  for(int i = 0; i < s.length(); ++i) {
    auto c = s[i];
    if(i >= 8) {
      break;
    }
    if(!seen[int(c)]) {
      s2 += c;
      seen[c] = true;
    }
  }
  auto N = s2.length();
  auto is_entr_coding = !(ui->radioLZ77->isChecked() || ui->radioLZW->isChecked());
  // set visibility for spinboxes and buttons
  ui->adjustCheckbox->setVisible(is_entr_coding);
  ui->btnAdjust->setVisible(is_entr_coding);
  ui->doubleSpinBox_1->setVisible(is_entr_coding && N >= 1);
  ui->doubleSpinBox_2->setVisible(is_entr_coding && N >= 2);
  ui->doubleSpinBox_3->setVisible(is_entr_coding && N >= 3);
  ui->doubleSpinBox_4->setVisible(is_entr_coding && N >= 4);
  ui->doubleSpinBox_5->setVisible(is_entr_coding && N >= 5);
  ui->doubleSpinBox_6->setVisible(is_entr_coding && N >= 6);
  ui->doubleSpinBox_7->setVisible(is_entr_coding && N >= 7);
  ui->doubleSpinBox_8->setVisible(is_entr_coding && N >= 8);
  ui->doubleSpinBox_EOT->setVisible(ui->radioArith->isChecked());
  current_alphabet_length = s2.length();
  if(s.length() != s2.length()) {
    ui->textAlphabet->document()->setPlainText(QString::fromStdString(s2));
  }
  update_input_text();
  adjust_probabilities();
  last_alphabet_length = s2.length();
}

// make sure probabilities add up to 1 and are non-negative
void MainWindow::adjust_probabilities() {
  auto &&alphabet_text = ui->textAlphabet->toPlainText().toStdString();
  auto len = alphabet_text.length();
  if(ui->radioArith->isChecked()) {
    ++len;
  }

  std::vector<QDoubleSpinBox *> spinboxes;
  if(ui->radioArith->isChecked()) {
    spinboxes.push_back(ui->doubleSpinBox_EOT);
    if(ui->doubleSpinBox_EOT->value() == .0f) {
      ui->doubleSpinBox_EOT->setValue(1.f / (len + 1));
    }
  }
  spinboxes.push_back(ui->doubleSpinBox_1);
  spinboxes.push_back(ui->doubleSpinBox_2);
  spinboxes.push_back(ui->doubleSpinBox_3);
  spinboxes.push_back(ui->doubleSpinBox_4);
  spinboxes.push_back(ui->doubleSpinBox_5);
  spinboxes.push_back(ui->doubleSpinBox_6);
  spinboxes.push_back(ui->doubleSpinBox_7);
  spinboxes.push_back(ui->doubleSpinBox_8);

  if(ui->adjustCheckbox->isChecked()) {
    const auto &&input_text = ui->textInput->toPlainText().toStdString();
    if(input_text.length()) {
      auto in_len = input_text.length();
      int count[256];
      for(int i = 0; i < 256; ++i) count[i] = 0;
      for(auto &c : input_text) {
        ++count[(unsigned char)(c)];
      }
      int start = 0;
      if(ui->radioArith->isChecked()) {
        start = 1;
        ++in_len;
        spinboxes[0]->setValue(1. / input_text.length() + 1);
      }
      for(int i = 0; i < len; ++i) {
        unsigned char a = alphabet_text[i];
        auto q = count[a];
        spinboxes[start + i]->setValue(double(q) / in_len);
      }
      if(start == 1) {
        auto p = 1. / in_len;
        if(p < 1e-3) {
            p = 2e-3;
        }
        spinboxes[0]->setValue(p);
      }
    }
  }

  auto sum = .0f;

  auto use_eot = ui->radioArith->isChecked() ? 1 : 0;
  for(int i = use_eot + last_alphabet_length; i < use_eot + current_alphabet_length; ++i) {
    if(i == 0) {
      spinboxes[i]->setValue(1.0f);
      continue;
    }
    spinboxes[i]->setValue(spinboxes[i - 1]->value());
  }
  for(int i = 0; i < len; ++i) {
    sum += spinboxes[i]->value();
  }
  if(sum == .0) {
    auto val = 1.f/len;
    for(int i = 0; i < len; ++i) {
      spinboxes[i]->setValue(val);
      sum += val;
    }
  } else if(!(std::fabs(1.f - sum) < 1e-6)) {
    for(int i = 0; i < len; ++i) {
      spinboxes[i]->setValue(spinboxes[i]->value() / sum);
    }
  }
  if(ui->radioArith->isChecked()) {
    if(spinboxes[0]->value() < 3e-3) {
      spinboxes[0]->setValue(3e-3);
    }
  } else {
    ui->doubleSpinBox_EOT->setValue(3e-3);
  }
}

// adjust probabilities
void MainWindow::on_btnAdjust_clicked() {
  adjust_probabilities();
  update_input_text();
}

// input changed
void MainWindow::on_textInput_textChanged() {
  adjust_probabilities();
  auto alphabet = ui->textAlphabet->toPlainText().toStdString();
  auto len = alphabet.length();

  // vector of probabilities
  std::vector<float> probs;
  // gather probabilities from the interface
  if(len >= 1) probs.push_back(ui->doubleSpinBox_1->value());
  if(len >= 2) probs.push_back(ui->doubleSpinBox_2->value());
  if(len >= 3) probs.push_back(ui->doubleSpinBox_3->value());
  if(len >= 4) probs.push_back(ui->doubleSpinBox_4->value());
  if(len >= 5) probs.push_back(ui->doubleSpinBox_5->value());
  if(len >= 6) probs.push_back(ui->doubleSpinBox_6->value());
  if(len >= 7) probs.push_back(ui->doubleSpinBox_7->value());
  if(len >= 8) probs.push_back(ui->doubleSpinBox_8->value());

  // remove duplicates
  std::string s = ui->textInput->toPlainText().toStdString();
  std::string s2;
  for(auto c : s) {
      if(alphabet.find(c) != std::string::npos && probs[alphabet.find(c)] != 0.) {
          s2 += c;
      }
  }
  if(s.length() != s2.length()) {
      ui->textInput->document()->setPlainText(QString::fromStdString((s2)));
      return;
  }

  // set statistical tools visibility
  auto is_entr_coding = !(ui->radioLZ77->isChecked() || ui->radioLZW->isChecked());
  ui->actPerfText->setVisible(is_entr_coding);
  ui->actualPerfLabel->setVisible(is_entr_coding);
  ui->entropyText->setVisible(is_entr_coding);
  ui->entropyLabel->setVisible(is_entr_coding);
  ui->avglenLabel->setVisible(is_entr_coding && !ui->radioArith->isChecked());
  ui->avglenText->setVisible(is_entr_coding && !ui->radioArith->isChecked());
  ui->optPerfText->setVisible(is_entr_coding);
  ui->optPerfLabel->setVisible(is_entr_coding);

  // empty alphabet
  if(len == 0) {
      if(is_entr_coding) {
        ui->entropyText->setText("nan");
        ui->optPerfText->setText(std::to_string(double(0)).c_str());
        ui->actPerfText->setText(std::to_string(double(0)).c_str());
        ui->avglenText->setText("nan");
      }
      return;
  }

  // end of text symbol
  if(ui->radioArith->isChecked()) {
    alphabet += coding::Arithmetic::END_OF_TEXT;
    probs.push_back(ui->doubleSpinBox_EOT->value());
  }

  // meta and source message
  const auto &&meta = coding::CodingMeta(alphabet, probs);
  const auto &&input = ui->textInput->toPlainText().toStdString();

  ui->labelInput->setText((std::string("Input (") + std::to_string(input.length()) + std::string(")")).c_str());

  // optimal performance and entropy
  double opt_perf = 0.;
  double entropy = 0.;
  for(int i = 0; i < meta.size(); ++i) {
    auto p = meta.get_prob(i);
    entropy -= p * std::log2(p);
  }
  for(auto c : input) {
    double p = meta.get_prob(meta.find_char(c));
    opt_perf -= std::log2(p);
  }
  if(ui->radioArith->isChecked()) {
    auto p = meta.get_prob(meta.find_char(coding::Arithmetic::END_OF_TEXT));
    opt_perf -= std::log2(p);
    entropy -= p * std::log2(p);
  }
  ui->entropyText->setText(std::to_string(entropy).c_str());
  ui->optPerfText->setText(std::to_string(opt_perf).c_str());

  DynamicBitset encoded;
  double avglen = -1;
  std::string decoded;
  if(input.length()) {
    if(ui->radioNoCoding->isChecked()) {
      coding::Base coder(meta);
      encoded = coder.encode(input);
      avglen = coder.average_length();
      decoded = coder.decode(encoded);
    } else if(ui->radioBlock->isChecked()) {
      coding::Block coder(meta);
      encoded = coder.encode(input);
      avglen = coder.average_length();
      decoded = coder.decode(encoded);
    } else if(ui->radioHuffman->isChecked()) {
      coding::Huffman coder(meta);
      encoded = coder.encode(input);
      avglen = coder.average_length();
      decoded = coder.decode(encoded);
    } else if(ui->radioArith->isChecked()) {
      coding::Arithmetic coder(meta);
      std::string text = input + coding::Arithmetic::END_OF_TEXT;
      encoded = coder.encode(text);
      decoded = coder.decode(encoded);
      decoded = decoded.substr(0, decoded.length() - 1);
    } else if(ui->radioShannon->isChecked()) {
      coding::Shannon coder(meta);
      encoded = coder.encode(input);
      avglen = coder.average_length();
      decoded = coder.decode(encoded);
    } else if(ui->radioLZ77->isChecked()) {
      coding::LZ77 coder(meta);
      encoded = coder.encode(input);
      decoded = coder.decode(encoded);
    } else if(ui->radioLZW->isChecked()) {
      coding::LZW coder(meta);
      encoded = coder.encode(input);
      decoded = coder.decode(encoded);
    }
  }
  if(avglen != -1 && !ui->radioArith->isChecked()) {
    ui->avglenText->setText(std::to_string(avglen).c_str());
  }

  ui->actPerfText->setText(std::to_string(double(encoded.size())).c_str());

  ui->textOutput->setText(QString::fromStdString(encoded.str()));
  ui->labelOutput->setText(QString::fromStdString(std::string() + "Encoded (" + std::to_string(encoded.size()) + ")"));
  ui->labelDecoded->setText((std::string("Decoded (") + std::to_string(decoded.length()) + std::string(")")).c_str());
  ui->textDecoded->setText(QString::fromStdString(decoded));
}


void MainWindow::on_radioNoCoding_clicked()
{
  update_alphabet_text();
}

void MainWindow::on_radioBlock_clicked()
{
  update_alphabet_text();
}

void MainWindow::on_radioHuffman_clicked()
{
  update_alphabet_text();
}

void MainWindow::on_radioArith_clicked()
{
  update_alphabet_text();
}

void MainWindow::on_radioShannon_clicked()
{
  update_alphabet_text();
}

void MainWindow::on_radioLZ77_clicked()
{
  update_alphabet_text();
}

void MainWindow::on_radioLZW_clicked()
{
  update_alphabet_text();
}

void MainWindow::on_adjustCheckbox_clicked()
{
  update_alphabet_text();
}
