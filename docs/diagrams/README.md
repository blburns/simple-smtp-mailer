# simple-smtp-mailer - Architecture and Flow Diagrams

This directory contains visual diagrams documenting the architecture and logic flow of simple-smtp-mailer.

## Available Diagrams

### Architecture Diagrams (`architecture.md`)
- **System Architecture**: High-level component diagram showing all layers and their relationships
- **Email Sending Flow**: Sequence diagram of email sending process
- **Queue Processing Flow**: Sequence diagram of queue management

### Flow Diagrams (`flow.md`)
- **Email Sending Flow**: Detailed flowchart of email sending logic
- **OAuth2 Authentication Flow**: Sequence diagram of OAuth2 token management
- **Queue Processing Flow**: Flowchart of queue processing and retry logic
- **API Provider Selection Flow**: Flowchart of provider selection and failover

## Diagram Format

All diagrams use [Mermaid](https://mermaid.js.org/) syntax, which can be rendered in:
- GitHub/GitLab markdown
- VS Code with Mermaid extension
- Most modern markdown viewers
- Online at [mermaid.live](https://mermaid.live)

## Usage

To view these diagrams:
1. **GitHub/GitLab**: Diagrams render automatically in markdown
2. **VS Code**: Install the "Markdown Preview Mermaid Support" extension
3. **Online**: Copy diagram code to [mermaid.live](https://mermaid.live)
4. **CLI**: Use `mmdc` (Mermaid CLI) to generate images

## Related Documentation

- [User Guide](../getting-started/README.md) - Getting started guide
- [Configuration Guide](../configuration/README.md) - Configuration options
- [API Documentation](../api/README.md) - API reference

