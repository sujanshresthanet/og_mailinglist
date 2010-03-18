<?php
// $Id$

require_once('og_mailinglist_utilities.inc');
require_once('og_mailinglist_api.inc');

function og_mailinglist_send_comment_email($comment, $node) {
  
  // Check if comment arrived via email so already sent.
  if (_og_mailinglist_email_already_sent($comment->nid, $comment->cid)) {
    return;
  }

  $mailer = og_mailinglist_create_mailer();

  $space = spaces_load('og', array_pop(array_values($node->og_groups)));  
  
  // Add custom headers.
  $mailer = og_mailinglist_add_web_headers($mailer, $space, $node);
  
  // Decorate subject.
  $mailer->Subject = "Re: [" . $space->purl . "] " . $node->title;

  // Decorate body.
  $body = og_mailinglist_prepare_web_content($comment->comment); // TODO figure out what the render function for nodes/comments is and use that instead. Current setup inconsistent plus won't shift to other's setups
  $body = og_mailinglist_build_web_footer($space, $node, $body);
  
  $mailer->Body = $body;
  $mailer->isHTML = TRUE;
  $mailer->AltBody = drupal_html_to_text($body);

  //// Add attachments if any.
  //foreach ($email['attachments'] as $info) { 
  //  $mailer->AddStringAttachment($info['data'], $info['filename']);
  //}
  
  $mailer = _og_mailinglist_add_addresses($mailer, $space, $node, FALSE,
                                 $comment);
  
  $success = $mailer->Send();
  
  if ($success) {
    og_mailinglist_log_email_sent('web', $node->nid, $comment->cid);
  }
  else {
    watchdog('og_mailinglist', "OG_Mailinglist couldn't send a new node email.", NULL,
             WATCHDOG_ERROR);
  }
  
  return;
}

function og_mailinglist_send_node_email($node) { 
  // Check if node arrived via email so already sent.
  if (_og_mailinglist_email_already_sent($node->nid)) {
    return;
  }
  
  // If not a group post, return. We only send out emails for group posts.
  if (!empty($node->og_groups)) {
    $space = spaces_load('og', array_pop(array_values((array) $node->og_groups)));   
  }
  else {
    return;
  }
  
  $mailer = og_mailinglist_create_mailer();
  
  // Add custom headers.
  $mailer = og_mailinglist_add_web_headers($mailer, $space, $node);

  // Decorate subject.
  $mailer->Subject = "[" . $space->purl . "] " . $node->title;

  // Decorate body.
  $body = og_mailinglist_prepare_web_content($node->body);
  $body = og_mailinglist_build_web_footer($space, $node, $body, 0, TRUE);

  
  $mailer->Body = $body;
  $mailer->isHTML = TRUE;
  $mailer->AltBody = drupal_html_to_text($body); 
  
  // Add attachments if any.
  if (isset($node->files)) {
    foreach ($node->files as $file) {
      $file = (object) $file;
      $mailer->AddAttachment($_SERVER['DOCUMENT_ROOT'] . base_path() .
                             $file->filepath, $file->filename);
    }
  }


  $mailer = _og_mailinglist_add_addresses($mailer, $space, $node, TRUE);
  
  $success = $mailer->Send();
  
  if ($success) {
    og_mailinglist_log_email_sent('web', $node->nid);
  }
  else {
    watchdog('og_mailinglist', "OG_Mailinglist couldn't send a new node email.", NULL,
             WATCHDOG_ERROR);
  }

  return;
}
