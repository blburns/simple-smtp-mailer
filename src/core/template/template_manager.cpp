#include "simple-smtp-mailer/template_manager.hpp"
#include <regex>
#include <algorithm>

namespace ssmtp_mailer {

TemplateManager::TemplateManager(std::shared_ptr<TemplateEngine> engine)
    : engine_(engine), total_templates_(0), total_renders_(0), total_errors_(0) {
}

bool TemplateManager::addTemplate(const EmailTemplate& template_data) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!engine_) {
        return false;
    }
    
    // Validate template
    if (!validateTemplate(template_data)) {
        total_errors_++;
        return false;
    }
    
    // Check if template already exists
    if (templates_.find(template_data.name) != templates_.end()) {
        return false;
    }
    
    // Add template
    auto template_ptr = std::make_shared<EmailTemplate>(template_data);
    templates_[template_data.name] = template_ptr;
    
    // Add to category index
    if (!template_data.category.empty()) {
        templates_by_category_[template_data.category].push_back(template_data.name);
    }
    
    total_templates_++;
    return true;
}

std::shared_ptr<EmailTemplate> TemplateManager::getTemplate(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = templates_.find(name);
    if (it != templates_.end()) {
        return it->second;
    }
    return nullptr;
}

bool TemplateManager::removeTemplate(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = templates_.find(name);
    if (it == templates_.end()) {
        return false;
    }
    
    // Remove from category index
    for (auto& category_pair : templates_by_category_) {
        auto& templates_in_category = category_pair.second;
        templates_in_category.erase(
            std::remove(templates_in_category.begin(), templates_in_category.end(), name),
            templates_in_category.end()
        );
    }
    
    templates_.erase(it);
    total_templates_--;
    return true;
}

std::vector<std::string> TemplateManager::listTemplates() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> template_names;
    for (const auto& pair : templates_) {
        template_names.push_back(pair.first);
    }
    
    return template_names;
}

std::vector<std::string> TemplateManager::listTemplatesByCategory(const std::string& category) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = templates_by_category_.find(category);
    if (it != templates_by_category_.end()) {
        return it->second;
    }
    return std::vector<std::string>();
}

std::shared_ptr<Email> TemplateManager::renderTemplate(const std::string& template_name,
                                                       const TemplateContext& context,
                                                       const std::string& from,
                                                       const std::vector<std::string>& to) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = templates_.find(template_name);
    if (it == templates_.end() || !engine_) {
        total_errors_++;
        return nullptr;
    }
    
    const auto& template_data = *it->second;
    
    try {
        // Render subject
        std::string subject = engine_->render(template_data.subject_template, context);
        
        // Render text body
        std::string text_body = engine_->render(template_data.text_body_template, context);
        
        // Render HTML body
        std::string html_body = engine_->render(template_data.html_body_template, context);
        
        // Create email
        auto email = std::make_shared<Email>();
        email->from = from;
        email->to = to;
        email->subject = subject;
        email->body = text_body;
        email->html_body = html_body;
        
        total_renders_++;
        return email;
        
    } catch (const std::exception& e) {
        total_errors_++;
        return nullptr;
    }
}

bool TemplateManager::renderTemplateToEmail(const std::string& template_name,
                                           const TemplateContext& context,
                                           Email& email) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = templates_.find(template_name);
    if (it == templates_.end() || !engine_) {
        total_errors_++;
        return false;
    }
    
    const auto& template_data = *it->second;
    
    try {
        // Render subject
        email.subject = engine_->render(template_data.subject_template, context);
        
        // Render text body
        email.body = engine_->render(template_data.text_body_template, context);
        
        // Render HTML body if template has it
        if (!template_data.html_body_template.empty()) {
            email.html_body = engine_->render(template_data.html_body_template, context);
        }
        
        total_renders_++;
        return true;
        
    } catch (const std::exception& e) {
        total_errors_++;
        return false;
    }
}

bool TemplateManager::validateTemplate(const EmailTemplate& template_data) {
    if (template_data.name.empty()) {
        return false;
    }
    
    if (template_data.subject_template.empty()) {
        return false;
    }
    
    if (template_data.text_body_template.empty()) {
        return false;
    }
    
    if (!engine_) {
        return false;
    }
    
    // Validate template syntax
    return engine_->validate(template_data.subject_template) &&
           engine_->validate(template_data.text_body_template) &&
           (template_data.html_body_template.empty() || engine_->validate(template_data.html_body_template));
}

std::map<std::string, int> TemplateManager::getStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::map<std::string, int> stats;
    stats["total_templates"] = total_templates_.load();
    stats["total_renders"] = total_renders_.load();
    stats["total_errors"] = total_errors_.load();
    stats["total_categories"] = templates_by_category_.size();
    
    return stats;
}

void TemplateManager::setTemplateEngine(std::shared_ptr<TemplateEngine> engine) {
    std::lock_guard<std::mutex> lock(mutex_);
    engine_ = engine;
}

std::shared_ptr<TemplateEngine> TemplateManager::getTemplateEngine() const {
    return engine_;
}

} // namespace ssmtp_mailer

