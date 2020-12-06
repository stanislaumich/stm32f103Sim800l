#include <Arduino.h>
void sendSMSinPDU(String phone, String message)
{
  Serial.println("Отправляем сообщение: " + message);

  // ============ Подготовка PDU-пакета =============================================================================================
  // В целях экономии памяти будем использовать указатели и ссылки
  String *ptrphone = &phone;                                    // Указатель на переменную с телефонным номером
  String *ptrmessage = &message;                                // Указатель на переменную с сообщением

  String PDUPack;                                               // Переменная для хранения PDU-пакета
  String *ptrPDUPack = &PDUPack;                                // Создаем указатель на переменную с PDU-пакетом

  int PDUlen = 0;                                               // Переменная для хранения длины PDU-пакета без SCA
  int *ptrPDUlen = &PDUlen;                                     // Указатель на переменную для хранения длины PDU-пакета без SCA

  getPDUPack(ptrphone, ptrmessage, ptrPDUPack, ptrPDUlen);      // Функция формирующая PDU-пакет, и вычисляющая длину пакета без SCA

  Serial.println("PDU-pack: " + PDUPack);
  Serial.println("PDU length without SCA:" + (String)PDUlen);

  // ============ Отправка PDU-сообщения ============================================================================================
  sendATCommand("AT+CMGF=0", true);                             // Включаем PDU-режим
  sendATCommand("AT+CMGS=" + (String)PDUlen, true);             // Отправляем длину PDU-пакета
  sendATCommand(PDUPack + (String)((char)26), true);            // После PDU-пакета отправляем Ctrl+Z
}

void getPDUPack(String *phone, String *message, String *result, int *PDUlen)
{
  // Поле SCA добавим в самом конце, после расчета длины PDU-пакета
  *result += "01";                                // Поле PDU-type - байт 00000001b
  *result += "00";                                // Поле MR (Message Reference)
  *result += getDAfield(phone, true);             // Поле DA
  *result += "00";                                // Поле PID (Protocol Identifier)
  *result += "08";                                // Поле DCS (Data Coding Scheme)
  //*result += "";                                // Поле VP (Validity Period) - не используется

  String msg = StringToUCS2(*message);            // Конвертируем строку в UCS2-формат

  *result += byteToHexString(msg.length() / 2);   // Поле UDL (User Data Length). Делим на 2, так как в UCS2-строке каждый закодированный символ представлен 2 байтами.
  *result += msg;

  *PDUlen = (*result).length() / 2;               // Получаем длину PDU-пакета без поля SCA
  *result = "00" + *result;                       // Добавляем поле SCA
}

String getDAfield(String *phone, bool fullnum) {
  String result = "";
  for (int i = 0; i <= (*phone).length(); i++) {  // Оставляем только цифры
    if (isDigit((*phone)[i])) {
      result += (*phone)[i];
    }
  }
  int phonelen = result.length();                 // Количество цифр в телефоне
  if (phonelen % 2 != 0) result += "F";           // Если количество цифр нечетное, добавляем F

  for (int i = 0; i < result.length(); i += 2) {  // Попарно переставляем символы в номере
    char symbol = result[i + 1];
    result = result.substring(0, i + 1) + result.substring(i + 2);
    result = result.substring(0, i) + (String)symbol + result.substring(i);
  }

  result = fullnum ? "91" + result : "81" + result; // Добавляем формат номера получателя, поле PR
  result = byteToHexString(phonelen) + result;    // Добавляем длиу номера, поле PL

  return result;
}

String sendATCommand(String cmd, bool waiting) {
  String _resp = "";                            // Переменная для хранения результата
  Serial.println(cmd);                          // Дублируем команду в монитор порта
  SIM800.println(cmd);                          // Отправляем команду модулю
  if (waiting) {                                // Если необходимо дождаться ответа...
    _resp = waitResponse();                     // ... ждем, когда будет передан ответ
    // Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
    if (_resp.startsWith(cmd)) {                // Убираем из ответа дублирующуюся команду
      _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
    }
    Serial.println(_resp);                      // Дублируем ответ в монитор порта
  }
  return _resp;                                 // Возвращаем результат. Пусто, если проблема
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                            // Переменная для хранения результата
  long _timeout = millis() + 10000;             // Переменная для отслеживания таймаута (10 секунд)
  while (!SIM800.available() && millis() < _timeout)  {}; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (SIM800.available()) {                     // Если есть, что считывать...
    _resp = SIM800.readString();                // ... считываем и запоминаем
  }
  else {                                        // Если пришел таймаут, то...
    Serial.println("Timeout...");               // ... оповещаем об этом и...
  }
  return _resp;                                 // ... возвращаем результат. Пусто, если проблема
}

// =================================== Блок декодирования UCS2 в читаемую строку UTF-8 =================================
String UCS2ToString(String s) {                       // Функция декодирования UCS2 строки
  String result = "";
  unsigned char c[5] = "";                            // Массив для хранения результата
  for (int i = 0; i < s.length() - 3; i += 4) {       // Перебираем по 4 символа кодировки
    unsigned long code = (((unsigned int)HexSymbolToChar(s[i])) << 12) +    // Получаем UNICODE-код символа из HEX представления
                         (((unsigned int)HexSymbolToChar(s[i + 1])) << 8) +
                         (((unsigned int)HexSymbolToChar(s[i + 2])) << 4) +
                         ((unsigned int)HexSymbolToChar(s[i + 3]));
    if (code <= 0x7F) {                               // Теперь в соответствии с количеством байт формируем символ
      c[0] = (char)code;
      c[1] = 0;                                       // Не забываем про завершающий ноль
    } else if (code <= 0x7FF) {
      c[0] = (char)(0xC0 | (code >> 6));
      c[1] = (char)(0x80 | (code & 0x3F));
      c[2] = 0;
    } else if (code <= 0xFFFF) {
      c[0] = (char)(0xE0 | (code >> 12));
      c[1] = (char)(0x80 | ((code >> 6) & 0x3F));
      c[2] = (char)(0x80 | (code & 0x3F));
      c[3] = 0;
    } else if (code <= 0x1FFFFF) {
      c[0] = (char)(0xE0 | (code >> 18));
      c[1] = (char)(0xE0 | ((code >> 12) & 0x3F));
      c[2] = (char)(0x80 | ((code >> 6) & 0x3F));
      c[3] = (char)(0x80 | (code & 0x3F));
      c[4] = 0;
    }
    result += String((char*)c);                       // Добавляем полученный символ к результату
  }
  return (result);
}

unsigned char HexSymbolToChar(char c) {
  if      ((c >= 0x30) && (c <= 0x39)) return (c - 0x30);
  else if ((c >= 'A') && (c <= 'F'))   return (c - 'A' + 10);
  else                                 return (0);
}

// =================================== Блок кодирования строки в представление UCS2 =================================
String StringToUCS2(String s)
{
  String output = "";                                               // Переменная для хранения результата

  for (int k = 0; k < s.length(); k++) {                            // Начинаем перебирать все байты во входной строке
    byte actualChar = (byte)s[k];                                   // Получаем первый байт
    unsigned int charSize = getCharSize(actualChar);                // Получаем длину символа - кличество байт.

    // Максимальная длина символа в UTF-8 - 6 байт плюс завершающий ноль, итого 7
    char symbolBytes[charSize + 1];                                 // Объявляем массив в соответствии с полученным размером
    for (int i = 0; i < charSize; i++)  symbolBytes[i] = s[k + i];  // Записываем в массив все байты, которыми кодируется символ
    symbolBytes[charSize] = '\0';                                   // Добавляем завершающий 0

    unsigned int charCode = symbolToUInt(symbolBytes);              // Получаем DEC-представление символа из набора байтов
    if (charCode > 0)  {                                            // Если все корректно преобразовываем его в HEX-строку
      // Остается каждый из 2 байт перевести в HEX формат, преобразовать в строку и собрать в кучу
      output += byteToHexString((charCode & 0xFF00) >> 8) +
                byteToHexString(charCode & 0xFF);
    }
    k += charSize - 1;                                              // Передвигаем указатель на начало нового символа
    if (output.length() >= 280) break;                              // Строка превышает 70 (4 знака на символ * 70 = 280) символов, выходим
  }
  return output;                                                    // Возвращаем результат
}

unsigned int getCharSize(unsigned char b) { // Функция получения количества байт, которыми кодируется символ
  // По правилам кодирования UTF-8, по старшим битам первого октета вычисляется общий размер символа
  // 1  0xxxxxxx - старший бит ноль (ASCII код совпадает с UTF-8) - символ из системы ASCII, кодируется одним байтом
  // 2  110xxxxx - два старших бита единицы - символ кодируется двумя байтами
  // 3  1110xxxx - 3 байта и т.д.
  // 4  11110xxx
  // 5  111110xx
  // 6  1111110x

  if (b < 128) return 1;             // Если первый байт из системы ASCII, то он кодируется одним байтом

  // Дальше нужно посчитать сколько единиц в старших битах до первого нуля - таково будет количество байтов на символ.
  // При помощи маски, поочереди исключаем старшие биты, до тех пор пока не дойдет до нуля.
  for (int i = 1; i <= 7; i++) {
    if (((b << i) & 0xFF) >> 7 == 0) {
      return i;
    }
  }
  return 1;
}

unsigned int symbolToUInt(const String& bytes) {  // Функция для получения DEC-представления символа
  unsigned int charSize = bytes.length();         // Количество байт, которыми закодирован символ
  unsigned int result = 0;
  if (charSize == 1) {
    return bytes[0]; // Если символ кодируется одним байтом, сразу отправляем его
  }
  else  {
    unsigned char actualByte = bytes[0];
    // У первого байта оставляем только значимую часть 1110XXXX - убираем в начале 1110, оставляем XXXX
    // Количество единиц в начале совпадает с количеством байт, которыми кодируется символ - убираем их
    // Например (для размера 2 байта), берем маску 0xFF (11111111) - сдвигаем её (>>) на количество ненужных бит (3 - 110) - 00011111
    result = actualByte & (0xFF >> (charSize + 1)); // Было 11010001, далее 11010001&(11111111>>(2+1))=10001
    // Каждый следующий байт начинается с 10XXXXXX - нам нужны только по 6 бит с каждого последующего байта
    // А поскольку остался только 1 байт, резервируем под него место:
    result = result << (6 * (charSize - 1)); // Было 10001, далее 10001<<(6*(2-1))=10001000000

    // Теперь у каждого следующего бита, убираем ненужные биты 10XXXXXX, а оставшиеся добавляем к result в соответствии с расположением
    for (int i = 1; i < charSize; i++) {
      actualByte = bytes[i];
      if ((actualByte >> 6) != 2) return 0; // Если байт не начинается с 10, значит ошибка - выходим
      // В продолжение примера, берется существенная часть следующего байта
      // Например, у 10011111 убираем маской 10 (биты в начале), остается - 11111
      // Теперь сдвигаем их на 2-1-1=0 сдвигать не нужно, просто добавляем на свое место
      result |= ((actualByte & 0x3F) << (6 * (charSize - 1 - i)));
      // Было result=10001000000, actualByte=10011111. Маской actualByte & 0x3F (10011111&111111=11111), сдвигать не нужно
      // Теперь "пристыковываем" к result: result|11111 (10001000000|11111=10001011111)
    }
    return result;
  }
}

String byteToHexString(byte i) { // Функция преобразования числового значения байта в шестнадцатиричное (HEX)
  String hex = String(i, HEX);
  if (hex.length() == 1) hex = "0" + hex;
  hex.toUpperCase();
  return hex;
}




