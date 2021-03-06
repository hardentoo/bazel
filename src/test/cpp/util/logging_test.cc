// Copyright 2016 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "src/main/cpp/blaze_util_platform.h"
#include "src/main/cpp/util/bazel_log_handler.h"
#include "src/main/cpp/util/logging.h"
#include "gtest/gtest.h"

namespace blaze_util {
class LoggingTest : public ::testing::Test {
 protected:
  void SetUp() {
    // Set the value of $TMP first, because CaptureStderr retrieves a temp
    // directory path and on Windows, the corresponding function (GetTempPathA)
    // reads $TMP.
    blaze::SetEnv("TMP", blaze::GetEnv("TEST_TMPDIR"));
  }
};

TEST(LoggingTest, BazelLogHandlerDumpsToCerrAtFail) {
  // Set up logging and be prepared to capture stderr at destruction.
  testing::internal::CaptureStderr();
  std::unique_ptr<blaze_util::BazelLogHandler> handler(
      new blaze_util::BazelLogHandler());
  blaze_util::SetLogHandler(std::move(handler));

  // Log something.
  std::string teststring = "test that the log messages get dumped to stderr";
  BAZEL_LOG(INFO) << teststring;

  // Check that stderr isn't getting anything yet.
  std::string stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) == std::string::npos)
      << "stderr unexpectedly contains the log message, before log output was "
         "set. Stderr contained: "
      << stderr_output;
  testing::internal::CaptureStderr();

  // Destruct the log handler and get the stderr remains.
  blaze_util::SetLogHandler(nullptr);
  stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) != std::string::npos)
      << "stderr does not contain the expected message. Stderr contained: "
      << stderr_output;
}

TEST(LoggingTest, LogLevelNamesMatch) {
  EXPECT_STREQ("INFO", LogLevelName(LOGLEVEL_INFO));
  EXPECT_STREQ("WARNING", LogLevelName(LOGLEVEL_WARNING));
  EXPECT_STREQ("ERROR", LogLevelName(LOGLEVEL_ERROR));
  EXPECT_STREQ("FATAL", LogLevelName(LOGLEVEL_FATAL));
}

TEST(LoggingTest, BazelLogDoesNotDumpToStderrIfOuputStreamSetToNull) {
  // Set up logging and be prepared to capture stderr at destruction.
  testing::internal::CaptureStderr();
  std::unique_ptr<blaze_util::BazelLogHandler> handler(
      new blaze_util::BazelLogHandler());
  blaze_util::SetLogHandler(std::move(handler));

  // Log something.
  std::string teststring = "test that the log message is lost.";
  BAZEL_LOG(INFO) << teststring;
  blaze_util::SetLoggingOutputStream(nullptr);

  // Destruct the log handler and check if stderr got anything.
  blaze_util::SetLogHandler(nullptr);
  std::string stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) == std::string::npos)
      << "stderr unexpectedly contains the log message, even though log output "
         "was explicitly set to null. Stderr contained: "
      << stderr_output;
}

TEST(LoggingTest, DirectLogsToBufferStreamWorks) {
  // Set up logging and be prepared to capture stderr at destruction.
  testing::internal::CaptureStderr();
  std::unique_ptr<blaze_util::BazelLogHandler> handler(
      new blaze_util::BazelLogHandler());
  blaze_util::SetLogHandler(std::move(handler));

  // Ask that the logs get output to a string buffer (keep a ptr to it so we can
  // check its contents)
  std::unique_ptr<std::stringstream> stringbuf(new std::stringstream());
  std::stringstream* stringbuf_ptr = stringbuf.get();
  blaze_util::SetLoggingOutputStream(std::move(stringbuf));

  std::string teststring = "testing log getting directed to a stringbuffer.";
  BAZEL_LOG(INFO) << teststring;

  // Check that output went to the buffer.
  std::string output(stringbuf_ptr->str());
  EXPECT_TRUE(output.find(teststring) != std::string::npos)
      << "log output is missing the log message, the output is: " << output;

  // Check that the output never went to stderr.
  std::string stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) == std::string::npos)
      << "stderr unexpectedly contains the log message that should have gone "
         "to the specified string buffer. Stderr contained: "
      << stderr_output;
}

TEST(LoggingTest, BufferedLogsSentToSpecifiedStream) {
  // Set up logging and be prepared to capture stderr at destruction.
  testing::internal::CaptureStderr();
  std::unique_ptr<blaze_util::BazelLogHandler> handler(
      new blaze_util::BazelLogHandler());
  blaze_util::SetLogHandler(std::move(handler));

  std::string teststring =
      "test sending logs to the buffer before setting the output stream";
  BAZEL_LOG(INFO) << teststring;

  // Check that stderr isn't getting anything yet.
  std::string stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) == std::string::npos)
      << "stderr unexpectedly contains the log message, before log output was "
         "set. Stderr contained: "
      << stderr_output;
  testing::internal::CaptureStderr();

  // Ask that the logs get output to a string buffer (keep a ptr to it so we can
  // check its contents)
  std::unique_ptr<std::stringstream> stringbuf(new std::stringstream());
  std::stringstream* stringbuf_ptr = stringbuf.get();
  blaze_util::SetLoggingOutputStream(std::move(stringbuf));

  // Check that the buffered logs were sent.
  std::string output(stringbuf_ptr->str());
  EXPECT_TRUE(output.find(teststring) != std::string::npos)
      << "log output is missing the log message, the output is: " << output;

  // Check that the output did not go to stderr.
  stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) == std::string::npos)
      << "stderr unexpectedly contains the log message that should have gone "
         "to the specified string buffer. Stderr contained: "
      << stderr_output;
}

TEST(LoggingTest, DirectLogsToCerrWorks) {
  // Set up logging and be prepared to capture stderr at destruction.
  testing::internal::CaptureStderr();
  std::unique_ptr<blaze_util::BazelLogHandler> handler(
      new blaze_util::BazelLogHandler());
  blaze_util::SetLogHandler(std::move(handler));

  // Ask that the logs get output to stderr
  blaze_util::SetLoggingOutputStreamToStderr();

  // Log something.
  std::string teststring = "test that the log messages get directed to cerr";
  BAZEL_LOG(INFO) << teststring;

  // Cause the logs to be flushed, and capture them.
  blaze_util::SetLogHandler(nullptr);
  std::string stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) != std::string::npos)
      << "stderr does not contain the expected log message. Stderr contained: "
      << stderr_output;
}

TEST(LoggingTest, BufferedLogsGetDirectedToCerr) {
  // Set up logging and be prepared to capture stderr at destruction.
  testing::internal::CaptureStderr();
  std::unique_ptr<blaze_util::BazelLogHandler> handler(
      new blaze_util::BazelLogHandler());
  blaze_util::SetLogHandler(std::move(handler));

  // Log something before telling the loghandler where to send it.
  std::string teststring = "test that this message gets directed to cerr";
  BAZEL_LOG(INFO) << teststring;

  // Ask that the logs get output to stderr
  blaze_util::SetLoggingOutputStreamToStderr();

  // Cause the logs to be flushed, and capture them.
  blaze_util::SetLogHandler(nullptr);
  std::string stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find(teststring) != std::string::npos)
      << "stderr does not contain the expected log message. Stderr contained: "
      << stderr_output;
}

TEST(LoggingTest, ImpossibleFile) {
  // Set up logging and be prepared to capture stderr at destruction.
  testing::internal::CaptureStderr();
  std::unique_ptr<blaze_util::BazelLogHandler> handler(
      new blaze_util::BazelLogHandler());
  blaze_util::SetLogHandler(std::move(handler));

  // Deliberately try to log to an impossible location, check that we error out.
  std::unique_ptr<std::ofstream> bad_logfile_stream_(
      new std::ofstream("/this/doesnt/exist.log", std::fstream::out));
  blaze_util::SetLoggingOutputStream(std::move(bad_logfile_stream_));

  // Cause the logs to be flushed, and capture them.
  blaze_util::SetLogHandler(nullptr);
  std::string stderr_output = testing::internal::GetCapturedStderr();
  EXPECT_TRUE(stderr_output.find("ERROR") != std::string::npos);
  EXPECT_TRUE(stderr_output.find("Provided stream failed") != std::string::npos)
      << "stderr does not contain the expected error. Stderr contained: "
      << stderr_output;
}
}  // namespace blaze_util
