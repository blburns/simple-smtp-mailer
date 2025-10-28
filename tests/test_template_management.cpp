#include <gtest/gtest.h>
#include "simple-smtp-mailer/template_manager.hpp"

class TemplateManagementTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create template engine
        engine_ = std::make_shared<ssmtp_mailer::SimpleTemplateEngine>();
        manager_ = std::make_unique<ssmtp_mailer::TemplateManager>(engine_);
        
        // Create test template
        test_template_.name = "test_template";
        test_template_.subject_template = "Hello {{name}}";
        test_template_.text_body_template = "Welcome {{name}}! This is a test.";
        test_template_.html_body_template = "<h1>Welcome {{name}}!</h1>";
        test_template_.category = "test";
        test_template_.version = "1.0";
    }

    void TearDown() override {
        manager_.reset();
    }

    std::shared_ptr<ssmtp_mailer::TemplateEngine> engine_;
    std::unique_ptr<ssmtp_mailer::TemplateManager> manager_;
    ssmtp_mailer::EmailTemplate test_template_;
};

// Test template manager creation
TEST_F(TemplateManagementTest, ManagerCreation) {
    EXPECT_NE(manager_, nullptr);
    EXPECT_NE(engine_, nullptr);
}

// Test adding template
TEST_F(TemplateManagementTest, AddTemplate) {
    EXPECT_TRUE(manager_->addTemplate(test_template_));
    
    // Try to add duplicate
    EXPECT_FALSE(manager_->addTemplate(test_template_));
}

// Test getting template
TEST_F(TemplateManagementTest, GetTemplate) {
    manager_->addTemplate(test_template_);
    
    auto template = manager_->getTemplate("test_template");
    EXPECT_NE(template, nullptr);
    EXPECT_EQ(template->name, "test_template");
    
    // Non-existent template
    auto missing = manager_->getTemplate("nonexistent");
    EXPECT_EQ(missing, nullptr);
}

// Test removing template
TEST_F(TemplateManagementTest, RemoveTemplate) {
    manager_->addTemplate(test_template_);
    
    EXPECT_TRUE(manager_->removeTemplate("test_template"));
    EXPECT_FALSE(manager_->removeTemplate("nonexistent"));
}

// Test listing templates
TEST_F(TemplateManagementTest, ListTemplates) {
    auto templates = manager_->listTemplates();
    EXPECT_TRUE(templates.empty());
    
    manager_->addTemplate(test_template_);
    templates = manager_->listTemplates();
    EXPECT_EQ(templates.size(), 1);
    EXPECT_EQ(templates[0], "test_template");
}

// Test listing by category
TEST_F(TemplateManagementTest, ListTemplatesByCategory) {
    manager_->addTemplate(test_template_);
    
    auto templates = manager_->listTemplatesByCategory("test");
    EXPECT_EQ(templates.size(), 1);
    
    auto empty = manager_->listTemplatesByCategory("nonexistent");
    EXPECT_TRUE(empty.empty());
}

// Test template validation
TEST_F(TemplateManagementTest, TemplateValidation) {
    // Valid template
    EXPECT_TRUE(manager_->validateTemplate(test_template_));
    
    // Invalid template (empty name)
    ssmtp_mailer::EmailTemplate invalid_template = test_template_;
    invalid_template.name = "";
    EXPECT_FALSE(manager_->validateTemplate(invalid_template));
}

// Test template rendering
TEST_F(TemplateManagementTest, TemplateRendering) {
    manager_->addTemplate(test_template_);
    
    ssmtp_mailer::TemplateContext context;
    context.addVariable("name", "John");
    context.addVariable("extra", "value");
    
    auto email = manager_->renderTemplate("test_template", context, 
                                          "sender@example.com", 
                                          {"recipient@example.com"});
    
    EXPECT_NE(email, nullptr);
    EXPECT_EQ(email->subject, "Hello John");
    EXPECT_EQ(email->body, "Welcome John! This is a test.");
    EXPECT_EQ(email->from, "sender@example.com");
    EXPECT_EQ(email->to[0], "recipient@example.com");
}

// Test template statistics
TEST_F(TemplateManagementTest, TemplateStatistics) {
    auto stats = manager_->getStatistics();
    EXPECT_EQ(stats["total_templates"], 0);
    
    manager_->addTemplate(test_template_);
    stats = manager_->getStatistics();
    EXPECT_EQ(stats["total_templates"], 1);
}

// Test template engine factory
TEST_F(TemplateManagementTest, TemplateEngineFactory) {
    auto engines = ssmtp_mailer::TemplateFactory::getSupportedEngines();
    EXPECT_FALSE(engines.empty());
    
    auto simple_engine = ssmtp_mailer::TemplateFactory::createEngine("Simple");
    EXPECT_NE(simple_engine, nullptr);
    
    auto handlebars_engine = ssmtp_mailer::TemplateFactory::createEngine("Handlebars");
    EXPECT_NE(handlebars_engine, nullptr);
}

